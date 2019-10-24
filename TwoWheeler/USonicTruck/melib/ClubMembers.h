// ClubMembers.h

#ifndef CLUBMEMBERS_H
#define CLUBMEMBERS_H

enum ClubMemberESP8266ID /*: unsigned long*/ {
	MEMBER_UNKNOWN = 0x00000000,
	MEMBER_CLAUDE  = 0x005dd8cc,
	MEMBER_ALYSSA  = 0x00716bd8,
	MEMBER_ELI     = 0x00715b1e,
	MEMBER_EMILY   = 0x00716cce,
	MEMBER_EUGENIE = 0x00715973,
	MEMBER_JAYLYN  = 0x001987cb,
	MEMBER_KEELER  = 0x00fc6a50,
	MEMBER_LEAH    = 0x007162b3,
	MEMBER_NATALIE = 0x0071687a,
	MEMBER_ROSE    = 0x001980ed,
};


ClubMemberESP8266ID mbridThisDevice() {
	return (ClubMemberESP8266ID)ESP.getChipId();
}

class ClubMember {
	protected:
		const ClubMemberESP8266ID m_mbrid;
		const char* const m_pszMemberName;
		const char* const m_pszSsid;
		const char* const m_pszPassword;
		const char* const m_pszMdnsName;
		const char* const m_pszOTAName;
		const char* const m_pszOTAPassword;

	public:
		ClubMember(
				const ClubMemberESP8266ID mbrid,
				const char* const pszMemberName,
				const char* const pszSsid,
				const char* const pszPassword,
				const char* const pszMdnsName,
				const char* const pszOTAName,
				const char* const pszOTAPassword
				) :
			m_mbrid(mbrid),
			m_pszMemberName(pszMemberName),
			m_pszSsid(pszSsid),
			m_pszPassword(pszPassword),
			m_pszMdnsName(pszMdnsName),
			m_pszOTAName(pszOTAName),
			m_pszOTAPassword(pszOTAPassword)
	{
	}


		ClubMemberESP8266ID mbrid() const { return m_mbrid; }
		const char* const pszMemberName() const { return m_pszMemberName; }
		const char* const pszSsid() const { return m_pszSsid; }
		const char* const pszPassword() const { return m_pszPassword; }
		const char* const pszMdnsName() const { return m_pszMdnsName; }
		const char* const pszOTAName() const { return m_pszOTAName; }
		const char* const pszOTAPassword() const { return m_pszOTAPassword; }

};

const ClubMember* const g_pmbrUnk = new ClubMember(MEMBER_UNKNOWN , "UNKNOWN_ESP8266", "Unknown member name", "Cl02539#aude", "Claude", "ClaudeProgram", "ClOTA1884aude");
const ClubMember* const g_rgpmbr [] = {
	new ClubMember(MEMBER_CLAUDE , "Claude", "Claude", "Cl02539#aude", "Claude", "ClaudeProgram", "ClOTA1884aude"),
	new ClubMember(MEMBER_ALYSSA , "Alyssa", "Alyssa", "Al02539#yssa", "Alyssa", "AlyssaProgram", "AlOTA1884yssa"),
	new ClubMember(MEMBER_ELI    , "Eli", "Eli", "El02539#i", "Eli", "EliProgram", "ElOTA1884i"),
	new ClubMember(MEMBER_EMILY  , "Emily", "Emily", "Em02539#ily", "Emily", "EmilyProgram", "EmOTA1884ily"),
	new ClubMember(MEMBER_EUGENIE, "Eugenie", "Eugenie", "Eu02539#genie", "Eugenie", "EugenieProgram", "EuOTA1884genie"),
	new ClubMember(MEMBER_JAYLYN , "Jaylyn", "Jaylyn", "Ja02539#ylyn", "Jaylyn", "JaylynProgram", "JaOTA1884ylyn"),
	new ClubMember(MEMBER_KEELER , "Keeler", "Keeler", "Ke02539#eler", "Keeler", "KeelerProgram", "KeOTA1884eler"),
	new ClubMember(MEMBER_LEAH   , "Leah", "Leah", "Le02539#ah", "Leah", "LeahProgram", "LeOTA1884ah"),
	new ClubMember(MEMBER_NATALIE, "Natalie", "Natalie", "Na02539#talie", "Natalie", "NatalieProgram", "NaOTA1884talie"),
	new ClubMember(MEMBER_ROSE   , "Rose", "Rose", "Ro02539#se", "Rose", "RoseProgram", "RoOTA1884se"),
};

const ClubMember* const pmbrOwnerOfThisESP8266() {
	const ClubMemberESP8266ID mbrid = mbridThisDevice();
	const int cpmbr = sizeof(g_rgpmbr) / sizeof(g_rgpmbr[0]);
	for(int ipmbr=0;ipmbr<cpmbr;ipmbr++) {
		const ClubMember* const pmbr = g_rgpmbr[ipmbr];
		if(pmbr->mbrid() == mbrid) {
			return pmbr;
		}
	}
	return g_pmbrUnk;
}

#endif //ndef CLUBMEMBERS_H
