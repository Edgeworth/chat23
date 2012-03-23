#include <algorithm>
#include <pcrecpp.h>
#include <glibmm/checksum.h>
#include "msnp8/msnp8.hpp"

namespace MSNP8 {
using sigc::signal;
using sigc::mem_fun;
using sigc::bind;

class ComNS {
public:
	static void setPhone(Ptr(Contact) cont, const std::string& id, const std::string& val, MSN& msn) {
		if (id == "PHH") cont->m_hp = val;
		else if (id == "PHW") cont->m_wp = val;
		else if (id == "PHM") cont->m_mp = val;
		else if (id == "MOB") cont->m_mob_auth = (val == "Y");
		else if (id == "MBE") cont->m_mob_exist = (val == "Y");
		else LOG(COMP("Unsupported mobile option '%1' on contact '%2'", val, cont->email()), LOG_WARN)
		LOG(COMP("Set '%1''s '%2' value to %3", cont->email(), id, val), LOG_MSG)
		msn.dispatch(bind(msn.on_cont.make_slot(), cont));
	}
	static void setStatus(Ptr(Contact) cont, const std::string& id, MSN& msn) {
		if (id == "NLN") cont->m_status = ONLINE;
		else if (id == "BSY") cont->m_status = BUSY;
		else if (id == "IDL") cont->m_status = IDLE;
		else if (id == "BRB") cont->m_status = BRB;
		else if (id == "AWY") cont->m_status = AWAY;
		else if (id == "PHN") cont->m_status = PHONE;
		else if (id == "LUN") cont->m_status = LUNCH;
		else if (id == "FLN") cont->m_status = OFFLINE;
		else LOG(COMP("Unsupported status '%1' on contact '%2'", id, cont->email()), LOG_WARN)
		LOG(COMP("Set '%1''s status to %2", cont->email(), id), LOG_MSG)
		msn.dispatch(bind(msn.on_cont.make_slot(), cont));
	}
	static void ns_XFR(Command& com, MSN& msn) {
		msn.m_chat.push_back(Ptr(Chat)(new Chat));
		msn.m_chat.back()->m_sb = tcp(com[3]);
		msn.m_chat.back()->m_sb->send(COMP("USR 1 %1 %2\r\n", msn.self()->email(), com[5]));
		if(!pcrecpp::RE("^USR 1 OK").PartialMatch(msn.m_chat.back()->m_sb->recvall())) {
				LOG("Could not connect to switchboard", LOG_ERR)
				return;
		}
		msn.dispatch(bind(msn.on_chat_create.make_slot(), msn.m_chat.back()));
	}
	static void ns_BLP(Command& com, MSN& msn) {}
	static void ns_BPR(Command& com, MSN& msn) {
		setPhone(msn.cont.back(), com[1], com[2], msn);
	}
	static void ns_ILN(Command& com, MSN& msn) {
		msn.contByEmail(com[3])->m_name = com[4];
		setStatus(msn.contByEmail(com[3]), com[2], msn);
	}
	static void ns_LSG(Command& com, MSN& msn) {
		msn.m_grp[strToInt(com[1])] = com[2];
		msn.dispatch(bind(msn.on_grp.make_slot(), strToInt(com[1])));
	}
	static void ns_LST(Command& com, MSN& msn) {
		msn.cont.push_back(Ptr(Contact)(new Contact));
		msn.cont.back()->m_email = com[1];
		msn.cont.back()->m_name = com[2];
		msn.contByEmail(com[1]) = msn.cont.back();

		int list;
		pcrecpp::RE("(\\d+)").PartialMatch(com[3], &list);
		msn.cont.back()->m_list.fl = list&FL;
		msn.cont.back()->m_list.al = list&AL;
		msn.cont.back()->m_list.bl = list&BL;
		msn.cont.back()->m_list.rl = list&RL;

		if (list&FL) {
			pcrecpp::StringPiece piece(com[4]);
			while (pcrecpp::RE("(\\d+)").FindAndConsume(&piece, &list))
				msn.cont.back()->m_grp.insert(list);
		}
		msn.dispatch(bind(msn.on_cont.make_slot(), msn.cont.back()));
	}
	static void ns_MSG(Command& com, MSN& msn) {}
	static void ns_PRP(Command& com, MSN& msn) {
		if (com.size() > 3) {
			com.erase(com.begin()+1, com.begin()+3);
			com.resize(3);
		}
		setPhone(msn.self(), com[1], com[2], msn);
	}
	static void ns_ADD(Command& com, MSN& msn) {
		Command lst(5);
		lst[0] = "LST";
		lst[1] = com[4];
		lst[2] = com[5];
		lst[4] = com[6];
		lst[3] = COMP("%d", (com[2] == "FL" ? FL : (com[2] == "AL" ? AL : BL)));
		ns_LST(lst, msn);
	}
	static void ns_ADG(Command& com, MSN& msn) {
		com.erase(com.begin()+1, com.begin()+3);
		std::swap(com[1], com[2]);
		ns_LSG(com, msn);
	}
	static void ns_CHG(Command& com, MSN& msn) {
		setStatus(msn.self(), com[2], msn);
	}
	static void ns_REA(Command& com, MSN& msn) {
		msn.self()->m_name = com[4];
		msn.dispatch(bind(msn.on_cont.make_slot(), msn.self()));
	}
	static void ns_REM(Command& com, MSN& msn) {
		if (com[2] == "AL") msn.contByEmail(com[4])->m_list.al = false;
		else {
			if (com.size() == 6) {
				msn.contByEmail(com[4])->m_grp.erase(strToInt(com[5]));
			} else {
				msn.contByEmail(com[4])->m_grp.clear();
				msn.contByEmail(com[4])->m_list.fl = false;
			}
		}
		msn.dispatch(bind(msn.on_cont.make_slot(), msn.contByEmail(com[4])));
	}
	static void ns_REG(Command& com, MSN& msn) {
		msn.m_grp[strToInt(com[3])] = com[4];
	}
	static void ns_RMG(Command& com, MSN& msn) {
		int gid = strToInt(com[3]);
		msn.m_grp.erase(gid);
		for (int i = 0; i < msn.cont.size(); ++i) {
			msn.cont[i]->m_grp.erase(gid);
			if (msn.cont[i]->m_grp.empty())
				msn.cont[i]->m_list.fl = false;
			msn.dispatch(bind(msn.on_cont.make_slot(), msn.cont[i]));
		}
	}
	static void ns_CHL(Command& com, MSN& msn) {
		msn.m_ns->send(COMP("QRY 1 msmsgs@msnmsgr.com 32\r\n%1",
			Glib::Checksum::compute_checksum(Glib::Checksum::CHECKSUM_MD5,
			com[2]+"Q1P7W2E4J9R8U3S5")));
	}
	static void ns_FLN(Command& com, MSN& msn) {
		setStatus(msn.contByEmail(com[1]), "FLN", msn);
	}

	static void ns_NLN(Command& com, MSN& msn) {
		msn.contByEmail(com[2])->m_name = com[3];
		setStatus(msn.contByEmail(com[2]), com[1], msn);
	}
	static void ns_RNG(Command& com, MSN& msn) {
		msn.m_chat.push_back(Ptr(Chat)(new Chat));
		msn.m_chat.back()->m_sb = tcp(com[2]);
		msn.m_chat.back()->m_sb->send(COMP("ANS 1 %1 %2 %3\r\n", msn.self()->email(), com[4], com[1]));
		msn.dispatch(bind(msn.on_chat_invite.make_slot(), msn.m_chat.back()));
	}
};

std::pair<std::string, sigc::slot<void, Command&, MSN&> > async_NS[] = {
		std::make_pair("XFR", &ComNS::ns_XFR),
		std::make_pair("BLP", &ComNS::ns_BLP),
		std::make_pair("BPR", &ComNS::ns_BPR),
		std::make_pair("ILN", &ComNS::ns_ILN),
		std::make_pair("LSG", &ComNS::ns_LSG),
		std::make_pair("LST", &ComNS::ns_LST),
		std::make_pair("MSG", &ComNS::ns_MSG),
		std::make_pair("PRP", &ComNS::ns_PRP),
		std::make_pair("ADD", &ComNS::ns_ADD),
		std::make_pair("ADG", &ComNS::ns_ADG),
		std::make_pair("CHG", &ComNS::ns_CHG),
		std::make_pair("REA", &ComNS::ns_REA),
		std::make_pair("REM", &ComNS::ns_REM),
		std::make_pair("REG", &ComNS::ns_REG),
		std::make_pair("RMG", &ComNS::ns_RMG),
		std::make_pair("RMG", &ComNS::ns_RMG),
		std::make_pair("CHL", &ComNS::ns_CHL),
		std::make_pair("FLN", &ComNS::ns_FLN),
		std::make_pair("NLN", &ComNS::ns_NLN),
		std::make_pair("RNG", &ComNS::ns_RNG)
};


size_t async_NS_size = sizeof(async_NS)/sizeof(*async_NS);
}
