#include "Database.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "Map.hpp"
#include "Sign.hpp"

namespace Database {

DB::DB()
	: db_(nullptr),
	  insertBlockStmt_(nullptr),
	  insertLightStmt_(nullptr),
	  insertSignStmt_(nullptr),
	  deleteSignStmt_(nullptr),
	  deleteSignsStmt_(nullptr),
	  loadBlocksStmt_(nullptr),
	  loadLightsStmt_(nullptr),
	  loadSignsStmt_(nullptr),
	  getKeyStmt_(nullptr),
	  setKeyStmt_(nullptr),
	  dbEnabled_(false),
	  workerRunning_(false),
	  ring_(1024) // capacité initiale du tampon
{
}

DB::~DB() {
	close();
}

void DB::enable() {
	dbEnabled_ = true;
}

void DB::disable() {
	dbEnabled_ = false;
}

bool DB::isEnabled() const {
	return dbEnabled_;
}

int DB::init(const std::string &path) {
	if (!dbEnabled_) return 0;
	// Requête de création (création de tables, index, etc.)
	static const char *createQuery =
		"attach database 'auth.db' as auth;"
		"create table if not exists auth.identity_token ("
		"   username text not null,"
		"   token text not null,"
		"   selected int not null"
		");"
		"create unique index if not exists auth.identity_token_username_idx on identity_token (username);"
		"create table if not exists state (x float not null, y float not null, z float not null, rx float not null, ry float not null);"
		"create table if not exists block (p int not null, q int not null, x int not null, y int not null, z int not null, w int not null);"
		"create table if not exists light (p int not null, q int not null, x int not null, y int not null, z int not null, w int not null);"
		"create table if not exists key (p int not null, q int not null, key int not null);"
		"create table if not exists sign (p int not null, q int not null, x int not null, y int not null, z int not null, face int not null, text text not null);"
		"create unique index if not exists block_pqxyz_idx on block (p, q, x, y, z);"
		"create unique index if not exists light_pqxyz_idx on light (p, q, x, y, z);"
		"create unique index if not exists key_pq_idx on key (p, q);"
		"create unique index if not exists sign_xyzface_idx on sign (x, y, z, face);"
		"create index if not exists sign_pq_idx on sign (p, q);";
	static const char *insertBlockQuery =
		"insert or replace into block (p, q, x, y, z, w) values (?, ?, ?, ?, ?, ?);";
	static const char *insertLightQuery =
		"insert or replace into light (p, q, x, y, z, w) values (?, ?, ?, ?, ?, ?);";
	static const char *insertSignQuery =
		"insert or replace into sign (p, q, x, y, z, face, text) values (?, ?, ?, ?, ?, ?, ?);";
	static const char *deleteSignQuery =
		"delete from sign where x = ? and y = ? and z = ? and face = ?;";
	static const char *deleteSignsQuery =
		"delete from sign where x = ? and y = ? and z = ?;";
	static const char *loadBlocksQuery =
		"select x, y, z, w from block where p = ? and q = ?;";
	static const char *loadLightsQuery =
		"select x, y, z, w from light where p = ? and q = ?;";
	static const char *loadSignsQuery =
		"select x, y, z, face, text from sign where p = ? and q = ?;";
	static const char *getKeyQuery =
		"select key from key where p = ? and q = ?;";
	static const char *setKeyQuery =
		"insert or replace into key (p, q, key) values (?, ?, ?);";

	int rc = sqlite3_open(path.c_str(), &db_);
	if (rc) return rc;
	rc = sqlite3_exec(db_, createQuery, nullptr, nullptr, nullptr);
	if (rc) return rc;
	rc = sqlite3_prepare_v2(db_, insertBlockQuery, -1, &insertBlockStmt_, nullptr);
	if (rc) return rc;
	rc = sqlite3_prepare_v2(db_, insertLightQuery, -1, &insertLightStmt_, nullptr);
	if (rc) return rc;
	rc = sqlite3_prepare_v2(db_, insertSignQuery, -1, &insertSignStmt_, nullptr);
	if (rc) return rc;
	rc = sqlite3_prepare_v2(db_, deleteSignQuery, -1, &deleteSignStmt_, nullptr);
	if (rc) return rc;
	rc = sqlite3_prepare_v2(db_, deleteSignsQuery, -1, &deleteSignsStmt_, nullptr);
	if (rc) return rc;
	rc = sqlite3_prepare_v2(db_, loadBlocksQuery, -1, &loadBlocksStmt_, nullptr);
	if (rc) return rc;
	rc = sqlite3_prepare_v2(db_, loadLightsQuery, -1, &loadLightsStmt_, nullptr);
	if (rc) return rc;
	rc = sqlite3_prepare_v2(db_, loadSignsQuery, -1, &loadSignsStmt_, nullptr);
	if (rc) return rc;
	rc = sqlite3_prepare_v2(db_, getKeyQuery, -1, &getKeyStmt_, nullptr);
	if (rc) return rc;
	rc = sqlite3_prepare_v2(db_, setKeyQuery, -1, &setKeyStmt_, nullptr);
	if (rc) return rc;
	sqlite3_exec(db_, "begin;", nullptr, nullptr, nullptr);
	workerStart();
	return 0;
}

void DB::close() {
	if (!dbEnabled_) return;
	workerStop();
	sqlite3_exec(db_, "commit;", nullptr, nullptr, nullptr);
	sqlite3_finalize(insertBlockStmt_);
	sqlite3_finalize(insertLightStmt_);
	sqlite3_finalize(insertSignStmt_);
	sqlite3_finalize(deleteSignStmt_);
	sqlite3_finalize(deleteSignsStmt_);
	sqlite3_finalize(loadBlocksStmt_);
	sqlite3_finalize(loadLightsStmt_);
	sqlite3_finalize(loadSignsStmt_);
	sqlite3_finalize(getKeyStmt_);
	sqlite3_finalize(setKeyStmt_);
	sqlite3_close(db_);
	db_ = nullptr;
}

void DB::commit() {
	if (!dbEnabled_) return;
	std::unique_lock<std::mutex> lock(mtx_);
	ring_.putCommit();
	cond_.notify_one();
}

void DB::execCommit() {
	sqlite3_exec(db_, "commit; begin;", nullptr, nullptr, nullptr);
}

void DB::authSet(const std::string &username, const std::string &identityToken) {
	if (!dbEnabled_) return;
	static const char *query =
		"insert or replace into auth.identity_token (username, token, selected) values (?, ?, ?);";
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
	sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, identityToken.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 3, 1);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	authSelect(username);
}

int DB::authSelect(const std::string &username) {
	if (!dbEnabled_) return 0;
	authSelectNone();
	static const char *query =
		"update auth.identity_token set selected = 1 where username = ?;";
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
	sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	return sqlite3_changes(db_);
}

void DB::authSelectNone() {
	if (!dbEnabled_) return;
	sqlite3_exec(db_, "update auth.identity_token set selected = 0;", nullptr, nullptr, nullptr);
}

bool DB::authGet(const std::string &username, std::string &identityToken) {
	if (!dbEnabled_) return false;
	static const char *query =
		"select token from auth.identity_token where username = ?;";
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
	sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
	bool found = false;
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		identityToken = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
		found = true;
	}
	sqlite3_finalize(stmt);
	return found;
}

bool DB::authGetSelected(std::string &username, std::string &identityToken) {
	if (!dbEnabled_) return false;
	static const char *query =
		"select username, token from auth.identity_token where selected = 1;";
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
	bool found = false;
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
		identityToken = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
		found = true;
	}
	sqlite3_finalize(stmt);
	return found;
}

void DB::saveState(float x, float y, float z, float rx, float ry) {
	if (!dbEnabled_) return;
	static const char *query =
		"insert into state (x, y, z, rx, ry) values (?, ?, ?, ?, ?);";
	sqlite3_stmt *stmt;
	sqlite3_exec(db_, "delete from state;", nullptr, nullptr, nullptr);
	sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
	sqlite3_bind_double(stmt, 1, x);
	sqlite3_bind_double(stmt, 2, y);
	sqlite3_bind_double(stmt, 3, z);
	sqlite3_bind_double(stmt, 4, rx);
	sqlite3_bind_double(stmt, 5, ry);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
}

bool DB::loadState(float &x, float &y, float &z, float &rx, float &ry) {
	if (!dbEnabled_) return false;
	static const char *query =
		"select x, y, z, rx, ry from state;";
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
	bool success = false;
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		x = sqlite3_column_double(stmt, 0);
		y = sqlite3_column_double(stmt, 1);
		z = sqlite3_column_double(stmt, 2);
		rx = sqlite3_column_double(stmt, 3);
		ry = sqlite3_column_double(stmt, 4);
		success = true;
	}
	sqlite3_finalize(stmt);
	return success;
}

void DB::insertBlock(int p, int q, int x, int y, int z, int w) {
	if (!dbEnabled_) return;
	std::unique_lock<std::mutex> lock(mtx_);
	ring_.putBlock(p, q, x, y, z, w);
	cond_.notify_one();
}

void DB::insertLight(int p, int q, int x, int y, int z, int w) {
	if (!dbEnabled_) return;
	std::unique_lock<std::mutex> lock(mtx_);
	ring_.putLight(p, q, x, y, z, w);
	cond_.notify_one();
}

void DB::insertSign(int p, int q, int x, int y, int z, int face, const std::string &text) {
	if (!dbEnabled_) return;
	sqlite3_reset(insertSignStmt_);
	sqlite3_bind_int(insertSignStmt_, 1, p);
	sqlite3_bind_int(insertSignStmt_, 2, q);
	sqlite3_bind_int(insertSignStmt_, 3, x);
	sqlite3_bind_int(insertSignStmt_, 4, y);
	sqlite3_bind_int(insertSignStmt_, 5, z);
	sqlite3_bind_int(insertSignStmt_, 6, face);
	sqlite3_bind_text(insertSignStmt_, 7, text.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_step(insertSignStmt_);
}

void DB::deleteSign(int x, int y, int z, int face) {
	if (!dbEnabled_) return;
	sqlite3_reset(deleteSignStmt_);
	sqlite3_bind_int(deleteSignStmt_, 1, x);
	sqlite3_bind_int(deleteSignStmt_, 2, y);
	sqlite3_bind_int(deleteSignStmt_, 3, z);
	sqlite3_bind_int(deleteSignStmt_, 4, face);
	sqlite3_step(deleteSignStmt_);
}

void DB::deleteSigns(int x, int y, int z) {
	if (!dbEnabled_) return;
	sqlite3_reset(deleteSignsStmt_);
	sqlite3_bind_int(deleteSignsStmt_, 1, x);
	sqlite3_bind_int(deleteSignsStmt_, 2, y);
	sqlite3_bind_int(deleteSignsStmt_, 3, z);
	sqlite3_step(deleteSignsStmt_);
}

void DB::deleteAllSigns() {
	if (!dbEnabled_) return;
	sqlite3_exec(db_, "delete from sign;", nullptr, nullptr, nullptr);
}

void DB::loadBlocks(World::Map &map, int p, int q) {
	if (!dbEnabled_) return;
	std::unique_lock<std::mutex> lock(loadMtx_);
	sqlite3_reset(loadBlocksStmt_);
	sqlite3_bind_int(loadBlocksStmt_, 1, p);
	sqlite3_bind_int(loadBlocksStmt_, 2, q);
	while (sqlite3_step(loadBlocksStmt_) == SQLITE_ROW) {
		int x = sqlite3_column_int(loadBlocksStmt_, 0);
		int y = sqlite3_column_int(loadBlocksStmt_, 1);
		int z = sqlite3_column_int(loadBlocksStmt_, 2);
		int w = sqlite3_column_int(loadBlocksStmt_, 3);
		map.set(x, y, z, w);
	}
}

void DB::loadLights(World::Map &map, int p, int q) {
	if (!dbEnabled_) return;
	std::unique_lock<std::mutex> lock(loadMtx_);
	sqlite3_reset(loadLightsStmt_);
	sqlite3_bind_int(loadLightsStmt_, 1, p);
	sqlite3_bind_int(loadLightsStmt_, 2, q);
	while (sqlite3_step(loadLightsStmt_) == SQLITE_ROW) {
		int x = sqlite3_column_int(loadLightsStmt_, 0);
		int y = sqlite3_column_int(loadLightsStmt_, 1);
		int z = sqlite3_column_int(loadLightsStmt_, 2);
		int w = sqlite3_column_int(loadLightsStmt_, 3);
		map.set(x, y, z, w);
	}
}

void DB::loadSigns(Utils::SignList &list, int p, int q) {
	if (!dbEnabled_) return;
	sqlite3_reset(loadSignsStmt_);
	sqlite3_bind_int(loadSignsStmt_, 1, p);
	sqlite3_bind_int(loadSignsStmt_, 2, q);
	while (sqlite3_step(loadSignsStmt_) == SQLITE_ROW) {
		int x = sqlite3_column_int(loadSignsStmt_, 0);
		int y = sqlite3_column_int(loadSignsStmt_, 1);
		int z = sqlite3_column_int(loadSignsStmt_, 2);
		int face = sqlite3_column_int(loadSignsStmt_, 3);
		const char *text = reinterpret_cast<const char*>(sqlite3_column_text(loadSignsStmt_, 4));
		list.add(x, y, z, face, text);
	}
}

int DB::getKey(int p, int q) {
	if (!dbEnabled_) return 0;
	sqlite3_reset(getKeyStmt_);
	sqlite3_bind_int(getKeyStmt_, 1, p);
	sqlite3_bind_int(getKeyStmt_, 2, q);
	if (sqlite3_step(getKeyStmt_) == SQLITE_ROW) {
		return sqlite3_column_int(getKeyStmt_, 0);
	}
	return 0;
}

void DB::setKey(int p, int q, int key) {
	if (!dbEnabled_) return;
	std::unique_lock<std::mutex> lock(mtx_);
	ring_.putKey(p, q, key);
	cond_.notify_one();
}

void DB::workerStart() {
	if (!dbEnabled_) return;
	workerRunning_ = true;
	workerThread_ = std::thread(&DB::workerRun, this);
}

void DB::workerStop() {
	if (!dbEnabled_) return;
	{
		std::unique_lock<std::mutex> lock(mtx_);
		ring_.putExit();
		cond_.notify_one();
	}
	if (workerThread_.joinable()) {
		workerThread_.join();
	}
}

void DB::workerRun() {
	bool running = true;
	while (running) {
		Utils::RingEntry e;
		{
			std::unique_lock<std::mutex> lock(mtx_);
			while (!ring_.get(e)) {
				cond_.wait(lock);
			}
		}
		switch (e.type) {
			case Utils::RingEntryType::BLOCK:
				// Appel de la fonction privée pour insérer un bloc.
				_db_insert_block(e.p, e.q, e.x, e.y, e.z, e.w);
				break;
			case Utils::RingEntryType::LIGHT:
				_db_insert_light(e.p, e.q, e.x, e.y, e.z, e.w);
				break;
			case Utils::RingEntryType::KEY:
				_db_set_key(e.p, e.q, e.key);
				break;
			case Utils::RingEntryType::COMMIT:
				execCommit();
				break;
			case Utils::RingEntryType::EXIT:
				running = false;
				break;
		}
	}
}

// Fonctions privées pour exécuter les requêtes en mode asynchrone.
void DB::_db_insert_block(int p, int q, int x, int y, int z, int w) {
	sqlite3_reset(insertBlockStmt_);
	sqlite3_bind_int(insertBlockStmt_, 1, p);
	sqlite3_bind_int(insertBlockStmt_, 2, q);
	sqlite3_bind_int(insertBlockStmt_, 3, x);
	sqlite3_bind_int(insertBlockStmt_, 4, y);
	sqlite3_bind_int(insertBlockStmt_, 5, z);
	sqlite3_bind_int(insertBlockStmt_, 6, w);
	sqlite3_step(insertBlockStmt_);
}

void DB::_db_insert_light(int p, int q, int x, int y, int z, int w) {
	sqlite3_reset(insertLightStmt_);
	sqlite3_bind_int(insertLightStmt_, 1, p);
	sqlite3_bind_int(insertLightStmt_, 2, q);
	sqlite3_bind_int(insertLightStmt_, 3, x);
	sqlite3_bind_int(insertLightStmt_, 4, y);
	sqlite3_bind_int(insertLightStmt_, 5, z);
	sqlite3_bind_int(insertLightStmt_, 6, w);
	sqlite3_step(insertLightStmt_);
}

void DB::_db_set_key(int p, int q, int key) {
	sqlite3_reset(setKeyStmt_);
	sqlite3_bind_int(setKeyStmt_, 1, p);
	sqlite3_bind_int(setKeyStmt_, 2, q);
	sqlite3_bind_int(setKeyStmt_, 3, key);
	sqlite3_step(setKeyStmt_);
}

} // namespace Database
