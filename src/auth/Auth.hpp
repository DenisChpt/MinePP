#ifndef AUTH_HPP
#define AUTH_HPP

#include <string>

namespace Auth {

	/**
	 * Obtient un jeton d'accès à partir des identifiants fournis.
	 * 
	 * @param result Chaîne dans laquelle sera stocké le jeton récupéré.
	 * @param username Nom d'utilisateur.
	 * @param identityToken Jeton d'identité préliminaire.
	 * @return true si l'authentification a réussi (code HTTP 200), false sinon.
	 */
	bool getAccessToken(std::string &result, 
						const std::string &username, 
						const std::string &identityToken);

} // namespace Auth

#endif // AUTH_HPP
