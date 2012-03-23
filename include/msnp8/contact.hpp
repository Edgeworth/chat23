#ifndef MSNP8_PRINCIPAL_HPP_
#define MSNP8_PRINCIPAL_HPP_
#include <set>
#include "common.hpp"

namespace MSNP8 {

enum Status {
	ONLINE,
	BUSY,
	IDLE,
	BRB,
	AWAY,
	PHONE,
	LUNCH,
	OFFLINE
};

enum List {
	FL = 1<<0,
	AL = 1<<1,
	BL = 1<<2,
	RL = 1<<3
};

class Contact {
public:
	Contact() : m_status(OFFLINE) {}

	Status status() {return m_status;}
	bool inGroup(int grp) {return (m_grp.find(grp) != m_grp.end());}
	const std::set<int>& groups() {return m_grp;}
	std::string email() {return m_email;}
	std::string name() {return m_name;}
	std::string homePhone() {return m_hp;}
	std::string workPhone() {return m_wp;}
	std::string mobPhone() {return m_mp;}
	bool hasMSNMobile() {return m_mob_exist;}
	bool allowMSNMobile() {return m_mob_auth;}
	bool onFL() {return m_list.fl;}
	bool onAL() {return m_list.al;}
	bool onBL() {return m_list.bl;}
	bool onRL() {return m_list.rl;}

private:

	Status m_status;
	std::set<int> m_grp;
	std::string m_email, m_name;
	std::string m_hp, m_wp, m_mp;
	bool m_mob_exist, m_mob_auth;
	struct {
		bool fl;
		bool al;
		bool bl;
		bool rl;
	} m_list;

	friend class MSN;
	friend class ComNS;
};

}

#endif
