/**
 * @file  LanguageSelect.cpp
 *
 * @brief Implements the Language Selection dialog class (which contains the language data)
 */
// ID line follows -- this is updated by SVN
// $Id$


#include "stdafx.h"
#include "merge.h"
#include "version.h"
#include "resource.h"
#include "LanguageSelect.h"
#include "BCMenu.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "DirFrame.h"
#include <locale.h>
#include <sstream>

// Escaped character constants in range 0x80-0xFF are interpreted in current codepage
// Using C locale gets us direct mapping to Unicode codepoints
#pragma setlocale("C")

// Select the translation system used:
// LANG_PO(LANG, PO) PO
// - for using PO files translation (without LANG files). Note that you'll need to
// create and compile MergeLang.dll.
// LANG_PO(LANG, PO) LANG
// - for using LANG files
#define LANG_PO(LANG, PO) PO

// Sanity-check definition of LANG_PO macro
#if LANG_PO(TRUE, FALSE) && !LANG_PO(FALSE, TRUE)
#pragma message("Compiling CLanguageSelect for use with .LANG files")
#elif LANG_PO(FALSE, TRUE) && !LANG_PO(TRUE, FALSE)
#pragma message("Compiling CLanguageSelect for use with .PO files")
#else
#error LANG_PO macro doesn't evaluate as expected
#endif

// RTL_NUMBER_OF should be defined in <winnt.h>
#ifndef RTL_NUMBER_OF
#define RTL_NUMBER_OF(A) (sizeof(A)/sizeof((A)[0]))
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/** @brief Relative path to WinMerge executable for lang files. */
static const TCHAR szRelativePath[] = _T("Languages\\");

static char *EatPrefix(char *text, const char *prefix);
static void unslash(unsigned codepage, std::string &s);

/////////////////////////////////////////////////////////////////////////////
// CLanguageSelect dialog

/** @brief Default English language. */
const WORD wSourceLangId = MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US);

/**
 * @brief Language definition.
 */
struct tLangDef
{
	UINT m_IdName; /**< ID of name in current UI language. */
	LPCWSTR m_NativeName; /**< Native language name. */
	LPCSTR m_AsciiName; /**< ASCII version of native name. */
	WORD  m_LangId; /**< Windows language identifier (LANGID). */
	const char *lang; /**< Name for language (LANG_CATALAN => CATALAN). */
	const char *sublang; /**< Name for sublang (SUBLANG_DEFAULT => DEFAULT). */
	LPCTSTR pszLocale; /**< Locale name to use. */
};

// Entry for languages for which we do not record a native name
// (either because we don't have one, or because the native name is the same as the ASCII
// name, which is in the string table resource -- eg, Italiano)
#define NONATIVE L""

// References:
// http://people.w3.org/rishida/names/languages.html
// http://www.vaelen.org/cgi-bin/vaelen/vaelen.cgi?topic=languagemenu-languagepacks

#define MAKELANGID2(lang, sublang) MAKELANGID(lang, sublang), #lang, #sublang

/**
 * @brief Language map.
 * @sa tLangDef.
 */
const tLangDef lang_map[] =
{
//	{IDS_AFRIKAANS, NONATIVE, MAKELANGID2(LANG_AFRIKAANS, SUBLANG_DEFAULT), _T("")},
	{IDS_ALBANIAN, L"Shqip", "Shqip", MAKELANGID2(LANG_ALBANIAN, SUBLANG_DEFAULT), _T("")},
//	{IDS_ARABIC_SAUDI, NONATIVE, MAKELANGID2(LANG_ARABIC, SUBLANG_ARABIC_SAUDI_ARABIA), _T("")},  
//	{IDS_ARABIC_IRAQ, NONATIVE, MAKELANGID2(LANG_ARABIC, SUBLANG_ARABIC_IRAQ), _T("")},  
	{IDS_ARABIC_EGYPT, L"\x0627\x0644\x0639\x0631\x0628\x064A\x0629", "Al Arabiya", MAKELANGID2(LANG_ARABIC, SUBLANG_ARABIC_EGYPT), _T("")},  
//	{IDS_ARABIC_LIBYA, NONATIVE, MAKELANGID2(LANG_ARABIC, SUBLANG_ARABIC_LIBYA), _T("")},  
//	{IDS_ARABIC_ALGERIA, NONATIVE, MAKELANGID2(LANG_ARABIC, SUBLANG_ARABIC_ALGERIA), _T("")},  
//	{IDS_ARABIC_MOROCCO, NONATIVE, MAKELANGID2(LANG_ARABIC, SUBLANG_ARABIC_MOROCCO), _T("")},  
//	{IDS_ARABIC_TUNISIA, NONATIVE, MAKELANGID2(LANG_ARABIC, SUBLANG_ARABIC_TUNISIA), _T("")},  
//	{IDS_ARABIC_OMAN, NONATIVE, MAKELANGID2(LANG_ARABIC, SUBLANG_ARABIC_OMAN), _T("")},  
//	{IDS_ARABIC_YEMEN, NONATIVE, MAKELANGID2(LANG_ARABIC, SUBLANG_ARABIC_YEMEN), _T("")},  
//	{IDS_ARABIC_SYRIA, NONATIVE, MAKELANGID2(LANG_ARABIC, SUBLANG_ARABIC_SYRIA), _T("")},  
//	{IDS_ARABIC_JORDAN, NONATIVE, MAKELANGID2(LANG_ARABIC, SUBLANG_ARABIC_JORDAN), _T("")},  
//	{IDS_ARABIC_LEBANON, NONATIVE, MAKELANGID2(LANG_ARABIC, SUBLANG_ARABIC_LEBANON), _T("")},  
//	{IDS_ARABIC_KUWAIT, NONATIVE, MAKELANGID2(LANG_ARABIC, SUBLANG_ARABIC_KUWAIT), _T("")},  
//	{IDS_ARABIC_UAE, NONATIVE, MAKELANGID2(LANG_ARABIC, SUBLANG_ARABIC_UAE), _T("")},  
//	{IDS_ARABIC_BAHRAIN, NONATIVE, MAKELANGID2(LANG_ARABIC, SUBLANG_ARABIC_BAHRAIN), _T("")},  
//	{IDS_ARABIC_QATAR, NONATIVE, MAKELANGID2(LANG_ARABIC, SUBLANG_ARABIC_QATAR), _T("")},  
	{IDS_ARMENIAN, L"\x540\x561\x575\x565\x580\x567\x576", "Hayeren", MAKELANGID2(LANG_ARMENIAN, SUBLANG_DEFAULT), _T("")},
//	{IDS_AZERI_LATIN, NONATIVE, MAKELANGID2(LANG_AZERI, SUBLANG_AZERI_LATIN), _T("")},
//	{IDS_AZERI_CYRILLIC, NONATIVE, MAKELANGID2(LANG_AZERI, SUBLANG_AZERI_CYRILLIC), _T("")},
	{IDS_BASQUE, L"Euskara", "Euskara", MAKELANGID2(LANG_BASQUE, SUBLANG_DEFAULT), _T("")},
	{IDS_BELARUSIAN, L"\x0411\x0435\x043B\x0430\x0440\x0443\x0441\x043A\x0430\x044F", "Belaruski", MAKELANGID2(LANG_BELARUSIAN, SUBLANG_DEFAULT), _T("")},
	{IDS_BULGARIAN, L"\x0411\x044A\x043B\x0433\x0430\x0440\x0441\x043A\x0438", "Bulgarian*", MAKELANGID2(LANG_BULGARIAN, SUBLANG_DEFAULT), _T("")},
	{IDS_CATALAN, L"Catal\xE0", "Catala", MAKELANGID2(LANG_CATALAN, SUBLANG_DEFAULT), _T("")},
	{IDS_CHINESE_TRADITIONAL, L"\x4E2d\x6587 (\x7E41\x9AD4)", "Zhongwen*", MAKELANGID2(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL), _T("cht")},
	//       &#20013;&#25991; (&#32321;&#39639;) [Chinese Traditional rendered in HTML Unicode codepoint escapes]
	{IDS_CHINESE_SIMPLIFIED, L"\x4E2D\x6587 (\x7B80\x4F53)", "Zhongwen*", MAKELANGID2(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), _T("chs")},
	//       &#20013;&#25991; (&#31616;&#20307;) [Chinese Simplified rendered in HTML Unicode codepoint escapes]
//	{IDS_CHINESE_HONGKONG, NONATIVE, MAKELANGID2(LANG_CHINESE, SUBLANG_CHINESE_HONGKONG), _T("chinese_hkg")},
//	{IDS_CHINESE_SINGAPORE, NONATIVE, MAKELANGID2(LANG_CHINESE, SUBLANG_CHINESE_SINGAPORE), _T("chinese_sgp")},
//	{IDS_CHINESE_MACAU, NONATIVE, MAKELANGID2(LANG_CHINESE, SUBLANG_CHINESE_MACAU), _T("")},
	{IDS_CROATIAN, L"Hrvatski", "Hrvatski", MAKELANGID2(LANG_CROATIAN, SUBLANG_DEFAULT), _T("")},
	{IDS_CZECH, L"\x010C" L"esk\xFD", "Cesko", MAKELANGID2(LANG_CZECH, SUBLANG_DEFAULT), _T("czech")},
	{IDS_DANISH, L"Dansk", "Dansk", MAKELANGID2(LANG_DANISH, SUBLANG_DEFAULT), _T("danish")},
	{IDS_DUTCH, L"Nederlands", "Nederlands", MAKELANGID2(LANG_DUTCH, SUBLANG_DUTCH), _T("dutch")},
//	{IDS_DUTCH_BELGIAN, NONATIVE, MAKELANGID2(LANG_DUTCH, SUBLANG_DUTCH_BELGIAN), _T("")},
	{IDS_ENGLISH_US, L"English", "English", MAKELANGID2(LANG_ENGLISH, SUBLANG_ENGLISH_US), _T("american")},
//	{IDS_ENGLISH_UK, NONATIVE, MAKELANGID2(LANG_ENGLISH, SUBLANG_ENGLISH_UK), _T("english-uk")},
//	{IDS_ENGLISH_AUS, NONATIVE, MAKELANGID2(LANG_ENGLISH, SUBLANG_ENGLISH_AUS), _T("australian")},  
//	{IDS_ENGLISH_CAN, NONATIVE, MAKELANGID2(LANG_ENGLISH, SUBLANG_ENGLISH_CAN), _T("canadian")},  
//	{IDS_ENGLISH_NZ, NONATIVE, MAKELANGID2(LANG_ENGLISH, SUBLANG_ENGLISH_NZ), _T("english-nz")},  
//	{IDS_ENGLISH_EIRE, NONATIVE, MAKELANGID2(LANG_ENGLISH, SUBLANG_ENGLISH_EIRE), _T("english_irl")},  
//	{IDS_ENGLISH_SOUTH_AFRICA, NONATIVE, MAKELANGID2(LANG_ENGLISH, SUBLANG_ENGLISH_SOUTH_AFRICA), _T("english")},  
//	{IDS_ENGLISH_JAMAICA, NONATIVE, MAKELANGID2(LANG_ENGLISH, SUBLANG_ENGLISH_JAMAICA), _T("english")},  
//	{IDS_ENGLISH_CARIBBEAN, NONATIVE, MAKELANGID2(LANG_ENGLISH, SUBLANG_ENGLISH_CARIBBEAN), _T("english")},  
//	{IDS_ENGLISH_BELIZE, NONATIVE, MAKELANGID2(LANG_ENGLISH, SUBLANG_ENGLISH_BELIZE), _T("english")},  
//	{IDS_ENGLISH_TRINIDAD, NONATIVE, MAKELANGID2(LANG_ENGLISH, SUBLANG_ENGLISH_TRINIDAD), _T("english")},  
//	{IDS_ENGLISH_ZIMBABWE, NONATIVE, MAKELANGID2(LANG_ENGLISH, SUBLANG_ENGLISH_ZIMBABWE), _T("english")},
//	{IDS_ENGLISH_PHILIPPINES, NONATIVE, MAKELANGID2(LANG_ENGLISH, SUBLANG_ENGLISH_PHILIPPINES), _T("english")},
	{IDS_ESTONIAN, L"Eesti", "Eesti", MAKELANGID2(LANG_ESTONIAN, SUBLANG_DEFAULT), _T("")},
	{IDS_FAEROESE, L"F\xF8" L"royskt", "F�royskt", MAKELANGID2(LANG_FAEROESE, SUBLANG_DEFAULT), _T("")},
	{IDS_FARSI, L"\x0641\x0627\x0631\x0633", "Farsi", MAKELANGID2(LANG_FARSI, SUBLANG_DEFAULT), _T("")},
	{IDS_FINNISH, L"Suomi", "Suomi", MAKELANGID2(LANG_FINNISH, SUBLANG_DEFAULT), _T("fin")},
	{IDS_FRENCH, L"Fran\xE7" L"ais", "Francais", MAKELANGID2(LANG_FRENCH, SUBLANG_FRENCH), _T("fra")},  
//	{IDS_FRENCH_BELGIAN, L"Fran\xE7" L"ais (Belgique)", MAKELANGID2(LANG_FRENCH, SUBLANG_FRENCH_BELGIAN), _T("frb")},   
//	{IDS_FRENCH_CANADIAN, L"Fran\xE7" L"ais (Canada)", MAKELANGID2(LANG_FRENCH, SUBLANG_FRENCH_CANADIAN), _T("frc")},   
//	{IDS_FRENCH_SWISS, L"Fran\xE7" L"ais (Suisse)", MAKELANGID2(LANG_FRENCH, SUBLANG_FRENCH_SWISS), _T("frs")},   
//	{IDS_FRENCH_LUXEMBOURG, L"Fran\xE7" L"ais (Luxembourg)", MAKELANGID2(LANG_FRENCH, SUBLANG_FRENCH_LUXEMBOURG), _T("french")},   
//	{IDS_FRENCH_MONACO, L"Fran\xE7" L"ais (Monaco)", MAKELANGID2(LANG_FRENCH, SUBLANG_FRENCH_MONACO), _T("")},
	{IDS_GEORGIAN, L"\x10E5\x10D0\x10E0\x10D7\x10E3\x10DA\x10D8", "Kartuli", MAKELANGID2(LANG_GEORGIAN, SUBLANG_DEFAULT), _T("")},
	{IDS_GERMAN, L"Deutsch", "Deutsch", MAKELANGID2(LANG_GERMAN, SUBLANG_GERMAN), _T("deu")}, 
//	{IDS_GERMAN_SWISS, NONATIVE, MAKELANGID2(LANG_GERMAN, SUBLANG_GERMAN_SWISS), _T("des")},  
//	{IDS_GERMAN_AUSTRIAN, NONATIVE, MAKELANGID2(LANG_GERMAN, SUBLANG_GERMAN_AUSTRIAN), _T("dea")},  
//	{IDS_GERMAN_LUXEMBOURG, NONATIVE, MAKELANGID2(LANG_GERMAN, SUBLANG_GERMAN_LUXEMBOURG), _T("deu")},  
//	{IDS_GERMAN_LIECHTENSTEIN, NONATIVE, MAKELANGID2(LANG_GERMAN, SUBLANG_GERMAN_LIECHTENSTEIN), _T("deu")},  
	{IDS_GREEK, L"\x0395\x03BB\x03BB\x03B7\x03BD\x03B9\x03BA\x03AC", "Ellenika", MAKELANGID2(LANG_GREEK, SUBLANG_DEFAULT), _T("greek")},
//	{IDS_HEBREW, L"\x05E2\x05D1\x05E8\x05D9\x05EA", MAKELANGID2(LANG_HEBREW, SUBLANG_DEFAULT), _T("")},
//	{IDS_HINDI, NONATIVE, MAKELANGID2(LANG_HINDI, SUBLANG_DEFAULT), _T("")},
	{IDS_HUNGARIAN, L"Magyar", "Magyar", MAKELANGID2(LANG_HUNGARIAN, SUBLANG_DEFAULT), _T("hun")},
//	{IDS_ICELANDIC, L"\xCDslenska", MAKELANGID2(LANG_ICELANDIC, SUBLANG_DEFAULT), _T("isl")},
//	{IDS_INDONESIAN, NONATIVE, MAKELANGID2(LANG_INDONESIAN, SUBLANG_DEFAULT), _T("")},
	{IDS_ITALIAN, L"Italiano", "Italiano", MAKELANGID2(LANG_ITALIAN, SUBLANG_ITALIAN), _T("ita")},
//	{IDS_ITALIAN_SWISS, NONATIVE, MAKELANGID2(LANG_ITALIAN, SUBLANG_ITALIAN_SWISS), _T("its")},
	// Kanji (ilbonhua in Hanja) (erbunhua in Mandarin)
	{IDS_JAPANESE, L"\x65E5\x672C\x8A9E", "Nihongo", MAKELANGID2(LANG_JAPANESE, SUBLANG_DEFAULT), _T("jpn")},
//	{IDS_KASHMIRI, NONATIVE, MAKELANGID2(LANG_KASHMIRI, SUBLANG_KASHMIRI_INDIA), _T("")},
//	{IDS_KAZAK, L"\x049A\x0430\x0437\x0430\x049B", MAKELANGID2(LANG_KAZAK, SUBLANG_DEFAULT), _T("")},
	// hangukhua in Hanja (should get this in Hangul ?) ? or "\xD55C\xAE00" ?
	// In Hangul, it is \xD55C\xaD6D\xC5B4 (HanGukO)
	{IDS_KOREAN, L"\x97D3\x56FD\x8A9E", "Hangul*", MAKELANGID2(LANG_KOREAN, SUBLANG_KOREAN), _T("kor")},
//	{IDS_LATVIAN, L"Latvie\x0161u", MAKELANGID2(LANG_LATVIAN, SUBLANG_DEFAULT), _T("")},
//	{IDS_LITHUANIAN, L"Lietuvi\x0161kai", MAKELANGID2(LANG_LITHUANIAN, SUBLANG_DEFAULT), _T("")},
//	{IDS_MALAY_MALAYSIA, NONATIVE, MAKELANGID2(LANG_MALAYALAM, SUBLANG_MALAY_MALAYSIA), _T("")},
//	{IDS_MALAY_BRUNEI_DARUSSALAM, NONATIVE, MAKELANGID2(LANG_MALAYALAM, SUBLANG_MALAY_BRUNEI_DARUSSALAM), _T("")},
//	{IDS_MANIPURI, NONATIVE, MAKELANGID2(LANG_MANIPURI, SUBLANG_DEFAULT), _T("")},
	{IDS_NORWEGIAN_BOKMAL, L"Norsk (Bokm\xE5l)", "Norsk (Bokmo)*", MAKELANGID2(LANG_NORWEGIAN, SUBLANG_NORWEGIAN_BOKMAL), _T("nor")},
//	{IDS_NORWEGIAN_NYNORSK, NONATIVE, MAKELANGID2(LANG_NORWEGIAN, SUBLANG_NORWEGIAN_NYNORSK), _T("non")},
	{IDS_POLISH, L"Polski", "Polski", MAKELANGID2(LANG_POLISH, SUBLANG_DEFAULT), _T("plk")},
	{IDS_PORTUGUESE, L"Portugu\x00EAs", "Portugues*", MAKELANGID2(LANG_PORTUGUESE, SUBLANG_PORTUGUESE), _T("ptg")},
	{IDS_PORTUGUESE_BRAZILIAN, L"Portugu\x00EAs brasileiro", "Portugues brasileiro*", MAKELANGID2(LANG_PORTUGUESE, SUBLANG_PORTUGUESE_BRAZILIAN), _T("ptb")},
//	{IDS_ROMANIAN, L"Rom\xE2n\x0103", MAKELANGID2(LANG_ROMANIAN, SUBLANG_DEFAULT), _T("")},
	//       Rom&#226;n&#259; [Romanian rendered in HTML Unicode codepoint escapes]
	{IDS_RUSSIAN, L"\x0440\x0443\x0441\x0441\x043A\x0438\x0439", "Ruskiyi*", MAKELANGID2(LANG_RUSSIAN, SUBLANG_DEFAULT), _T("rus")},
	//       &#1088;&#1091;&#1089;&#1089;&#1082;&#1080;&#1081; [Russian rendered in HTML Unicode codepoint escapes]
//	{IDS_SANSKRIT, NONATIVE, MAKELANGID2(LANG_SANSKRIT, SUBLANG_DEFAULT), _T("")},
	{IDS_SERBIAN_LATIN, L"Srpski", "Srpski", MAKELANGID2(LANG_SERBIAN, SUBLANG_SERBIAN_LATIN), _T("")},
	{IDS_SERBIAN_CYRILLIC, L"\x0421\x0440\x043F\x0441\x043A\x0438", "srpski", MAKELANGID2(LANG_SERBIAN, SUBLANG_SERBIAN_CYRILLIC), _T("")},
//	{IDS_SINDHI, NONATIVE, MAKELANGID2(LANG_SINDHI, SUBLANG_DEFAULT), _T("")},
	{IDS_SLOVAK, L"Sloven\x010Dina", "Slovencina*", MAKELANGID2(LANG_SLOVAK, SUBLANG_DEFAULT), _T("sky")},
	{IDS_SLOVENIAN, L"Sloven\x0161\x010Dina", "Slovenscina*", MAKELANGID2(LANG_SLOVENIAN, SUBLANG_DEFAULT), _T("")},
//	{IDS_SPANISH, L"Espa\xF1ol", MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH), _T("esm")}, 
//	{IDS_SPANISH_MEXICAN, NONATIVE, MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH_MEXICAN), _T("esp")}, 
	{IDS_SPANISH_MODERN, L"Espa\xF1ol (moderno)", "Espanol (moderno)", MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH_MODERN), _T("esn")}, 
//	{IDS_SPANISH_GUATEMALA, NONATIVE, MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH_GUATEMALA), _T("esp")}, 
//	{IDS_SPANISH_COSTA_RICA, NONATIVE, MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH_COSTA_RICA), _T("esp")}, 
//	{IDS_SPANISH_PANAMA, NONATIVE, MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH_PANAMA), _T("esp")}, 
//	{IDS_SPANISH_DOMINICAN, NONATIVE, MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH_DOMINICAN_REPUBLIC), _T("esp")}, 
//	{IDS_SPANISH_VENEZUELA, NONATIVE, MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH_VENEZUELA), _T("esp")}, 
//	{IDS_SPANISH_COLOMBIA, NONATIVE, MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH_COLOMBIA), _T("esp")}, 
//	{IDS_SPANISH_PERU, NONATIVE, MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH_PERU), _T("esp")}, 
//	{IDS_SPANISH_ARGENTINA, NONATIVE, MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH_ARGENTINA), _T("esp")}, 
//	{IDS_SPANISH_ECUADOR, NONATIVE, MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH_ECUADOR), _T("esp")}, 
//	{IDS_SPANISH_CHILE, NONATIVE, MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH_CHILE), _T("esp")}, 
//	{IDS_SPANISH_URUGUAY, NONATIVE, MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH_URUGUAY), _T("esp")}, 
//	{IDS_SPANISH_PARAGUAY, NONATIVE, MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH_PARAGUAY), _T("esp")}, 
//	{IDS_SPANISH_BOLIVIA, NONATIVE, MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH_BOLIVIA), _T("esp")}, 
//	{IDS_SPANISH_EL_SALVADOR, NONATIVE, MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH_EL_SALVADOR), _T("esp")}, 
//	{IDS_SPANISH_HONDURAS, NONATIVE, MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH_HONDURAS), _T("esp")}, 
//	{IDS_SPANISH_NICARAGUA, NONATIVE, MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH_NICARAGUA), _T("esp")}, 
//	{IDS_SPANISH_PUERTO_RICO, NONATIVE, MAKELANGID2(LANG_SPANISH, SUBLANG_SPANISH_PUERTO_RICO), _T("esp")}, 
//	{IDS_SWAHILI, NONATIVE, MAKELANGID2(LANG_SWAHILI, SUBLANG_DEFAULT), _T("")},
	{IDS_SWEDISH, L"Svenska", "Svenska", MAKELANGID2(LANG_SWEDISH, SUBLANG_SWEDISH), _T("sve")},
//	{IDS_SWEDISH_FINLAND, NONATIVE, MAKELANGID2(LANG_SWEDISH, SUBLANG_SWEDISH_FINLAND), _T("sve")},
//	{IDS_TAMIL, NONATIVE, MAKELANGID2(LANG_TAMIL, SUBLANG_DEFAULT), _T("")},
//	{IDS_TATAR, NONATIVE, MAKELANGID2(LANG_TATAR, SUBLANG_DEFAULT), _T("")},
//	{IDS_THAI, L"\x0E20\x0E32\x0E29\x0E32\x0E44\x0E17\x0E22", MAKELANGID2(LANG_THAI, SUBLANG_DEFAULT), _T("")},
	{IDS_TURKISH, L"T\x03CBrk\xE7" L"e", "Turkce", MAKELANGID2(LANG_TURKISH, SUBLANG_DEFAULT), _T("trk")},
//	{IDS_UKRANIAN, L"\x0423\x043A\x0440\x0430\x0457\x043D\x0441\x044C\x043A\x0430", MAKELANGID2(LANG_UKRAINIAN, SUBLANG_DEFAULT), _T("")},
	{IDS_URDU_PAKISTAN, L"\x0627\x0631\x062F\x0648", "Urdu (Pakistan)", MAKELANGID2(LANG_URDU, SUBLANG_URDU_PAKISTAN), _T("")},
	{IDS_URDU_INDIA, L"\x0627\x0631\x062F\x0648", "Urdu (India)", MAKELANGID2(LANG_URDU, SUBLANG_URDU_INDIA), _T("")},
//	{IDS_UZBEK_LATIN, NONATIVE, MAKELANGID2(LANG_UZBEK, SUBLANG_UZBEK_LATIN), _T("")},
//	{IDS_UZBEK_CYRILLIC, L"\x040E\x0437\x0431\x0435\x043A", MAKELANGID2(LANG_UZBEK, SUBLANG_UZBEK_CYRILLIC), _T("")},
//	{IDS_VIETNAMESE, L"Ti\xEA\x0301ng Vi\xEA\x0323t", MAKELANGID2(LANG_VIETNAMESE, SUBLANG_DEFAULT), _T("")},
	{0, L"0", "0", 0, NULL},
};

/**
 * @brief Finds language from language mep.
 * @param [in] lang Language name to find.
 * @param [in] sublang Sub-language name to find.
 * @return Index to language map if found, -1 if not found.
 */
static int GetLanguageArrayIndex(const char *lang, const char *sublang)
{
	for (int i = 0 ; lang_map[i].m_LangId != 0 ; ++i)
		if (strcmp(lang_map[i].lang, lang) == 0 &&
			strcmp(lang_map[i].sublang, sublang) == 0)
			return i;
	return -1;
}

CLanguageSelect::CLanguageSelect(UINT idMainMenu, UINT idDocMenu, BOOL bReloadMenu /*=TRUE*/, BOOL bUpdateTitle /*=TRUE*/, CWnd* pParent /*=NULL*/)
: CDialog(CLanguageSelect::IDD, pParent)
, m_hCurrentDll(0)
, m_pLog(0)
, m_wCurLanguage(wSourceLangId)
, m_idMainMenu(idMainMenu)
, m_idDocMenu(idDocMenu)
, m_hModule(NULL)
, m_bReloadMenu(bReloadMenu)
, m_bUpdateTitle(bUpdateTitle)
{
}


void CLanguageSelect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLanguageSelect)
	DDX_Control(pDX, IDC_LANGUAGE_LIST, m_ctlLangList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLanguageSelect, CDialog)
//{{AFX_MSG_MAP(CLanguageSelect)
	ON_LBN_DBLCLK(IDC_LANGUAGE_LIST, OnDblclkLanguageList)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/**
 * @brief Select the language.
 * @param [in] wLangId ID of language to select.
 * @param [in] override If true, save to registry.
 * @return TRUE if succeeded, FALSE otherwise.
 */
BOOL CLanguageSelect::SetLanguage(WORD wLangId, bool override)
{ 
	BOOL result = FALSE;

	// use local resources
	if ((PRIMARYLANGID(wLangId)== LANG_ENGLISH)
		&& (SUBLANGID(wLangId) == SUBLANG_ENGLISH_US))
	{  
		LoadResourceDLL();
		result = TRUE;
	}
	// use resources from DLL
	else
	{  
		if ( m_wCurLanguage != wLangId)
		{
			CString strPath = GetDllName(wLangId);
			
			if (!strPath.IsEmpty()
				&& LoadResourceDLL(strPath) )
			{
				result = TRUE;
			}
		}
	}

	if (result)
	{
		m_wCurLanguage = wLangId;
		if (override)
    		AfxGetApp()->WriteProfileInt( LANGUAGE_SECTION, COUNTRY_ENTRY, (INT) wLangId );
		SetThreadLocale(MAKELCID(m_wCurLanguage, SORT_DEFAULT));

		int idx = GetLanguageArrayIndex(m_wCurLanguage);
		if (idx != -1
			&& *lang_map[idx].pszLocale != _T('\0'))
		{
			_tsetlocale(LC_ALL, lang_map[idx].pszLocale);
		}
	}
	return result;
}

/**
 * @brief Remove prefix from the string.
 * @param [in] text String from which to jump over prefix.
 * @param [in] prefix Prefix string to jump over.
 * @return String without the prefix.
 * @note Function returns pointer to original string,
 *  it does not allocate a new string.
 */
static char *EatPrefix(char *text, const char *prefix)
{
	if (int len = strlen(prefix))
		if (_memicmp(text, prefix, len) == 0)
			return text + len;
	return 0;
}

/**
 * @brief Convert C style \nnn, \r, \n, \t etc into their indicated characters.
 * @param [in, out] s String to convert.
 */
static void unslash(unsigned codepage, std::string &s)
{
	char *p = &*s.begin();
	char *q = p;
	char c;
	do
	{
		char *r = q + 1;
		switch (c = *q)
		{
		case '\\':
			switch (c = *r++)
			{
			case 'a':
				c = '\a';
				break;
			case 'b':
				c = '\b';
				break;
			case 'f':
				c = '\f';
				break;
			case 'n':
				c = '\n';
				break;
			case 'r':
				c = '\r';
				break;
			case 't':
				c = '\t';
				break;
			case 'v':
				c = '\v';
				break;
			case 'x':
				*p = (char)strtol(r, &q, 16);
				break;
			default:
				*p = (char)strtol(--r, &q, 8);
				break;
			}
			if (q > r)
				break;
			// fall through
		default:
			*p = c;
			if ((*p & 0x80) && IsDBCSLeadByteEx(codepage, *p))
				*++p = *r++;
			q = r;
		}
		++p;
	} while (c != '\0');
	s.resize(p - 1 - &*s.begin());
}

/**
 * @brief Load & configure WinMerge to use resources from specified DLL.
 * @param [in] szDllFileName Full path to the language file to load.
 * @return TRUE when loading succeeds, FALSE otherwise.
 */
BOOL CLanguageSelect::LoadResourceDLL(LPCTSTR szDllFileName /*=NULL*/) 
{
	// reset the resource handle to point to the current file
	AfxSetResourceHandle(AfxGetInstanceHandle( ));
	
	// free the existing DLL
	if ( m_hCurrentDll != NULL )
	{
		FreeLibrary(m_hCurrentDll);
		m_hCurrentDll = NULL;
	}
	
	// bail if using local resources
	if (szDllFileName == NULL
		|| *szDllFileName == _T('\0'))
		return FALSE;
	
	// load the DLL
	if (m_pLog != NULL)
		m_pLog->Write(_T("Loading resource DLL: %s"), szDllFileName);

#if LANG_PO(TRUE, FALSE) // compiling for use with .LANG files
	m_hCurrentDll = LoadLibrary(szDllFileName);
	if (m_hCurrentDll == 0)
	{
		if (m_hWnd)
		{
			std_tchar(ostringstream) stm;
			stm << _T("Failed to load ") << szDllFileName;
			AfxMessageBox(stm.str().c_str(), MB_ICONSTOP);
		}
		return FALSE;
	}
#else // compiling for use with .PO files
	m_strarray.clear();
	m_codepage = 0;
	m_hCurrentDll = LoadLibrary(_T("MergeLang.dll"));
	// There is no point in translating error messages about inoperational
	// translation system, so go without string resources here.
	if (m_hCurrentDll == 0)
	{
		if (m_hWnd)
			AfxMessageBox(_T("Failed to load MergeLang.dll"), MB_ICONSTOP);
		return FALSE;
	}
	CVersionInfo viInstance = AfxGetInstanceHandle();
	DWORD instanceVerMS = 0;
	DWORD instanceVerLS = 0;
	viInstance.GetFixedFileVersion(instanceVerMS, instanceVerLS);
	CVersionInfo viResource = m_hCurrentDll;
	DWORD resourceVerMS = 0;
	DWORD resourceVerLS = 0;
	viResource.GetFixedFileVersion(resourceVerMS, resourceVerLS);
	if (instanceVerMS != resourceVerMS || instanceVerLS != resourceVerLS)
	{
		FreeLibrary(m_hCurrentDll);
		m_hCurrentDll = 0;
		if (m_hWnd)
			AfxMessageBox(_T("MergeLang.dll version mismatch"), MB_ICONSTOP);
		return FALSE;
	}
	HRSRC mergepot = FindResource(m_hCurrentDll, _T("MERGEPOT"), RT_RCDATA);
	if (mergepot == 0)
	{
		if (m_hWnd)
			AfxMessageBox(_T("MergeLang.dll is invalid"), MB_ICONSTOP);
		return FALSE;
	}
	size_t size = SizeofResource(m_hCurrentDll, mergepot);
	const char *data = (const char *)LoadResource(m_hCurrentDll, mergepot);
	char buf[1024];
	std::string *ps = 0;
	std::string msgid;
	std::vector<unsigned> lines;
	while (const char *eol = (const char *)memchr(data, '\n', size))
	{
		size_t len = eol - data;
		if (len >= sizeof buf)
		{
			ASSERT(FALSE);
			break;
		}
		memcpy(buf, data, len);
		buf[len++] = '\0';
		data += len;
		size -= len;
		if (char *p = EatPrefix(buf, "#:"))
		{
			if (char *q = strchr(p, ':'))
			{
				int line = strtol(q + 1, &q, 10);
				lines.push_back(line);
			}
		}
		else if (EatPrefix(buf, "msgid "))
		{
			ps = &msgid;
		}
		if (ps)
		{
			char *p = strchr(buf, '"');
			char *q = strrchr(buf, '"');
			if (std::string::size_type n = q - p)
			{
				ps->append(p + 1, n - 1);
			}
			else
			{
				ps = 0;
				for (unsigned *pline = &*lines.begin() ; pline < &*lines.end() ; ++pline)
				{
					unsigned line = *pline;
					if (m_strarray.size() <= line)
						m_strarray.resize(line + 1);
					m_strarray[line] = msgid;
				}
				lines.clear();
				msgid.erase();
			}
		}
	}
	FILE *f = _tfopen(szDllFileName, _T("r"));
	if (f == 0)
	{
		FreeLibrary(m_hCurrentDll);
		m_hCurrentDll = 0;
		if (m_hWnd)
		{
			std_tchar(ostringstream) stm;
			stm << _T("Failed to load ") << szDllFileName;
			AfxMessageBox(stm.str().c_str(), MB_ICONSTOP);
		}
		return FALSE;
	}
	ps = 0;
	msgid.erase();
	lines.clear();
	std::string format;
	std::string msgstr;
	std::string directive;
	int badrefs = 0;
	while (fgets(buf, sizeof buf, f))
	{
		if (char *p = EatPrefix(buf, "#:"))
		{
			if (char *q = strchr(p, ':'))
			{
				int line = strtol(q + 1, &q, 10);
				lines.push_back(line);
			}
		}
		else if (char *p = EatPrefix(buf, "#,"))
		{
			format = p;
			format.erase(0, format.find_first_not_of(" \t\r\n"));
			format.erase(format.find_last_not_of(" \t\r\n") + 1);
		}
		else if (char *p = EatPrefix(buf, "#."))
		{
			directive = p;
			directive.erase(0, directive.find_first_not_of(" \t\r\n"));
			directive.erase(directive.find_last_not_of(" \t\r\n") + 1);
		}
		else if (EatPrefix(buf, "msgid "))
		{
			ps = &msgid;
		}
		else if (EatPrefix(buf, "msgstr "))
		{
			ps = &msgstr;
		}
		if (ps)
		{
			char *p = strchr(buf, '"');
			char *q = strrchr(buf, '"');
			if (std::string::size_type n = q - p)
			{
				ps->append(p + 1, n - 1);
			}
			else
			{
				ps = 0;
				if (msgstr.empty())
					msgstr = msgid;
				unslash(m_codepage, msgstr);
				for (unsigned *pline = &*lines.begin() ; pline < &*lines.end() ; ++pline)
				{
					unsigned line = *pline;
					if (m_strarray.size() <= line)
						m_strarray.resize(line + 1);
					if (m_strarray[line] == msgid)
						m_strarray[line] = msgstr;
					else
						++badrefs;
				}
				lines.clear();
				if (directive == "Codepage")
				{
					m_codepage = strtol(msgstr.c_str(), &p, 10);
				}
				msgid.erase();
				msgstr.erase();
			}
		}
	}
	fclose(f);
	if (badrefs)
	{
		FreeLibrary(m_hCurrentDll);
		m_hCurrentDll = 0;
		m_strarray.clear();
		m_codepage = 0;
		if (m_hWnd)
		{
			std_tchar(ostringstream) stm;
			stm << _T("Mismatched references detected in ") << szDllFileName;
			AfxMessageBox(stm.str().c_str(), MB_ICONSTOP);
		}
		return FALSE;
	}
#endif // LANG_PO(TRUE, FALSE)
	AfxSetResourceHandle(m_hCurrentDll);
	return TRUE;
}


/**
 * @brief Convert specified Language ID into resource filename, if we have one for it
 */
CString CLanguageSelect::GetDllName( WORD wLangId ) 
{
	TCHAR fullpath[MAX_PATH+1];
	
	if ( GetModuleFileName(m_hModule, fullpath, _MAX_PATH ))
	{
		CStringArray dlls;
		WORD wDllLang;
		
		CString strPath = GetLanguagePath(fullpath);
		GetDllsAt(strPath, dlls);
		
		for (int i = 0; i < dlls.GetSize(); i++)
		{
			if (GetLanguage( dlls[i], wDllLang))
			{
				if (wLangId == wDllLang)
					return dlls[i];
			}
		}
	}
	
	return CString(_T(""));
}


/**
 * @brief Load array with all Merge language resource files at given directory
 */
void CLanguageSelect::GetDllsAt(LPCTSTR szSearchPath, CStringArray& dlls )
{
	WIN32_FIND_DATA ffi;
	CString strFileSpec;
	
	strFileSpec.Format(_T("%s*.") LANG_PO(_T("lang"), _T("po")), szSearchPath);
	HANDLE hff = FindFirstFile(strFileSpec, &ffi);
	
	if (  hff != INVALID_HANDLE_VALUE )
	{
		do
		{
			if (!(ffi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{

				strFileSpec.Format(_T("%s%s"), szSearchPath, ffi.cFileName);
				if (m_pLog != NULL)
					m_pLog->Write(_T("Found LANG file: %s"), strFileSpec);
				dlls.Add(strFileSpec);  
			}
		}
		while (FindNextFile(hff, &ffi));
		FindClose(hff);
	}
}

static CWordArray foundLangs;

BOOL CALLBACK EnumResLangProc(HANDLE /*hModule*/,	// module handle
							  LPCTSTR /*lpszType*/,  // pointer to resource type
							  LPCTSTR /*lpszName*/,  // pointer to resource name
							  WORD wIDLanguage,  // resource language identifier
							  LPARAM /*lParam*/)		// application-defined parameter)
{
	
	foundLangs.Add(wIDLanguage);
	return TRUE;
}


BOOL CLanguageSelect::GetLanguage( const CString& DllName, WORD& uiLanguage )
{
	BOOL bResult = FALSE;

#if LANG_PO(TRUE, FALSE) // compiling for use with .LANG files
	DWORD   dwVerInfoSize;		// Size of version information block
	DWORD   dwVerHnd=0;			// An 'ignored' parameter, always '0'
	CString s(DllName);
	LPTSTR pszFilename = s.GetBuffer(MAX_PATH);
	LPTSTR   m_lpstrVffInfo;	
	m_lpstrVffInfo = NULL;

	uiLanguage = wSourceLangId;
	dwVerInfoSize = GetFileVersionInfoSize(pszFilename, &dwVerHnd);
	if (dwVerInfoSize) {
		HANDLE  hMem;
		hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		m_lpstrVffInfo  = (LPTSTR)GlobalLock(hMem);
		if (GetFileVersionInfo(pszFilename, dwVerHnd, dwVerInfoSize, m_lpstrVffInfo))
		{
			LPWORD langInfo;
			DWORD langLen;
			if (VerQueryValue((LPVOID)m_lpstrVffInfo,
				_T("\\VarFileInfo\\Translation"),
				(LPVOID *)&langInfo, (UINT *)&langLen))
			{
				uiLanguage = langInfo[0];
				bResult = TRUE;
			}
		}
		GlobalUnlock(hMem);
		GlobalFree(hMem);
	}
#else // compiling for use with .PO files
	if (FILE *f = _tfopen(DllName, _T("r")))
	{
		char buf[1024];
		while (fgets(buf, sizeof buf, f))
		{
			int i = 0;
			strcat(buf, "1");
			sscanf(buf, "msgid \" LANG_ENGLISH , SUBLANG_ENGLISH_US \" %d", &i);
			if (i)
			{
				if (fgets(buf, sizeof buf, f))
				{
					char *lang = strstr(buf, "LANG_");
					char *sublang = strstr(buf, "SUBLANG_");
					strtok(lang, ",\" \t\r\n");
					strtok(sublang, ",\" \t\r\n");
					i = ::GetLanguageArrayIndex(lang, sublang);
					if (i != -1)
					{
						uiLanguage = lang_map[i].m_LangId;
						bResult = TRUE;
					}
				}
				break;
			}
		}
		fclose(f);
	}
#endif
	return bResult;
}

typedef long GetDllLangProc();
/*BOOL CLanguageSelect::GetLanguage( const CString& DllName, WORD& uiLanguage ) 
{
	BOOL bRes = FALSE;
	HINSTANCE hInst = LoadLibrary(DllName);
	
	if ( hInst )
	{										 
		foundLangs.SetSize(0,1);
		if (EnumResourceLanguages(hInst,			 // resource-module handle
			RT_DIALOG,			  // pointer to resource type
			MAKEINTRESOURCE(30000),			  // pointer to resource name
			(ENUMRESLANGPROC)EnumResLangProc,  // pointer to callback function
			0L))				  // application-defined parameter
		{
			if (m_pLog != NULL)
				m_pLog->Write(_T("%d languages found in file %s"), foundLangs.GetSize(), DllName);
			if (foundLangs.GetSize()>0)
			{
				if (m_pLog != NULL)
				{
					for (int i=0; i < foundLangs.GetSize(); i++)
						m_pLog->Write(_T("Found language: %s"), GetLanguageString(foundLangs.GetAt(i)));
				}
				uiLanguage = foundLangs.GetAt(0);
			bRes = TRUE;
		}
		}
		else
			uiLanguage = 0;
		
		FreeLibrary(hInst);
	}
	
	return bRes;
}*/

/**
 * @brief Return path part of fully qualified filename
 */
CString CLanguageSelect::GetPath( LPCTSTR FileName) const
{
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_PATH];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	
	_tsplitpath( FileName, drive, dir, fname, ext );
	CString Path = drive;
	Path += dir;

	if (Path.Right(1) != _T('\\')
		&& Path.Right(1) != _T('/'))
		Path += _T('\\');

	return Path;
}

/**
 * @brief Build Language subdirectory from fully qualified exe filename
 */
CString CLanguageSelect::GetLanguagePath(LPCTSTR FileName) const
{
	CString Path = GetPath(FileName);
	Path += szRelativePath;
	return Path;
}

/**
 * @brief Check if there are language files installed.
 *
 * This function does as fast as possible check for installed language
 * files. It needs to be fast since it is used in enabling/disabling
 * GUI item(s). So the simple check we do is just find one .lang file.
 * If there is a .lang file we assume we have at least one language
 * installed.
 * @return TRUE if at least one lang file is found. FALSE if no lang
 * files are found.
 */
BOOL CLanguageSelect::AreLangsInstalled() const
{
	WIN32_FIND_DATA ffi;
	CString strFileSpec;
	BOOL bFound = FALSE;
	TCHAR fullpath[MAX_PATH] = {0};

	if (GetModuleFileName(m_hModule, fullpath, _MAX_PATH))
	{
		CString strSearchPath = GetLanguagePath(fullpath);

		strFileSpec.Format(_T("%s*.") LANG_PO(_T("lang"), _T("po")), strSearchPath);
		HANDLE hff = FindFirstFile(strFileSpec, &ffi);

		if (hff != INVALID_HANDLE_VALUE)
		{
			// Found a .lang item, check it is a file
			if (!(ffi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				bFound = TRUE;
			}
			FindClose(hff);
		}
	}
	return bFound;
}

void CLanguageSelect::GetAvailLangs( CWordArray& wLanguageAry,
									CStringArray& DllFileNames ) 
{
	CString strPath;
	TCHAR filespec[MAX_PATH+1];
	WORD wLanguage;
	
	if ( GetModuleFileName(m_hModule, filespec, _MAX_PATH ))
	{
		strPath = GetLanguagePath(filespec);
		CStringArray dlls;
		
		GetDllsAt(strPath, dlls );
		
		for ( int i = 0; i < dlls.GetSize(); i++ )
		{
			if ( GetLanguage( dlls[i], wLanguage ) )
			{
				wLanguageAry.Add(wLanguage);
				DllFileNames.Add(dlls[i]);
			}
			else if (m_pLog != NULL)
				m_pLog->Write(_T("No languages found in file %s"), dlls[i]);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CLanguageSelect commands

bool CLanguageSelect::TranslateString(size_t line, std::string &s) const
{
#if LANG_PO(FALSE, TRUE) // compiling for use with .PO files
	if (line > 0 && line < m_strarray.size())
	{
		s = m_strarray[line];
		return true;
	}
#endif
	return false;
}

bool CLanguageSelect::TranslateString(size_t line, std::wstring &ws) const
{
#if LANG_PO(FALSE, TRUE) // compiling for use with .PO files
	if (line > 0 && line < m_strarray.size())
	{
		if (int len = m_strarray[line].length())
		{
			ws.resize(len);
			const char *msgstr = m_strarray[line].c_str();
			len = MultiByteToWideChar(m_codepage, 0, msgstr, -1, &*ws.begin(), len + 1);
			ASSERT(*msgstr == 0 || len != 0);
			ws.resize(len - 1);
			return true;
		}
	}
#endif
	return false;
}

void CLanguageSelect::SetIndicators(CStatusBar &sb, const UINT *rgid, int n) const
{
	HGDIOBJ hf = (HGDIOBJ)sb.SendMessage(WM_GETFONT);
	CClientDC dc(0);
	if (hf)
		hf = dc.SelectObject(hf);
	if (n)
		sb.SetIndicators(0, n);
	else
		n = sb.m_nCount;
	int cx = ::GetSystemMetrics(SM_CXSCREEN) / 4;	// default to 1/4 the screen width
	UINT style = SBPS_STRETCH | SBPS_NOBORDERS;		// first pane is stretchy
	for (int i = 0 ; i < n ; ++i)
	{
		UINT id = rgid ? rgid[i] : sb.GetItemID(i);
		if (id >= ID_INDICATOR_EXT)
		{
			String text = LoadString(id);
			int cx = dc.GetTextExtent(text.c_str(), text.length()).cx;
			sb.SetPaneInfo(i, id, style | SBPS_DISABLED, cx);
			sb.SetPaneText(i, text.c_str(), FALSE);
		}
		else if (rgid)
		{
			sb.SetPaneInfo(i, 0, style, cx);
		}
		style = 0;
	}
	if (hf)
		hf = dc.SelectObject(hf);
	// Send WM_SIZE to get pane rectangles right
	RECT rect;
	sb.GetClientRect(&rect);
	sb.SendMessage(WM_SIZE, 0, MAKELPARAM(rect.right, rect.bottom));
}

void CLanguageSelect::TranslateMenu(HMENU h) const
{
#if LANG_PO(FALSE, TRUE) // compiling for use with .PO files
	int i = ::GetMenuItemCount(h);
	while (i > 0)
	{
		--i;
		UINT id = 0;
		MENUITEMINFO mii;
		mii.cbSize = sizeof mii;
		mii.fMask = MIIM_STATE|MIIM_ID|MIIM_SUBMENU|MIIM_DATA;
		::GetMenuItemInfo(h, i, TRUE, &mii);
		if (mii.hSubMenu)
		{
			TranslateMenu(mii.hSubMenu);
			mii.wID = reinterpret_cast<UINT>(mii.hSubMenu);
		}
		if (BCMenuData *pItemData = reinterpret_cast<BCMenuData *>(mii.dwItemData))
		{
			if (LPCWSTR text = pItemData->GetWideString())
			{
				unsigned line = 0;
				swscanf(text, L"Merge.rc:%u", &line);
				std::wstring s;
				if (TranslateString(line, s))
					pItemData->SetWideString(s.c_str());
			}
		}
		TCHAR text[80];
		if (::GetMenuString(h, i, text, RTL_NUMBER_OF(text), MF_BYPOSITION))
		{
			unsigned line = 0;
			_stscanf(text, _T("Merge.rc:%u"), &line);
			String s;
			if (TranslateString(line, s))
				::ModifyMenu(h, i, mii.fState | MF_BYPOSITION, mii.wID, s.c_str());
		}
	}
#endif
}

void CLanguageSelect::TranslateDialog(HWND h) const
{
#if LANG_PO(FALSE, TRUE) // compiling for use with .PO files
	UINT gw = GW_CHILD;
	do
	{
		TCHAR text[80];
		::GetWindowText(h, text, RTL_NUMBER_OF(text));
		unsigned line = 0;
		_stscanf(text, _T("Merge.rc:%u"), &line);
		String s;
		if (TranslateString(line, s))
			::SetWindowText(h, s.c_str());
		h = ::GetWindow(h, gw);
		gw = GW_HWNDNEXT;
	} while (h);
#endif
}

String CLanguageSelect::LoadString(UINT id) const
{
	String s;
	if (id)
	{
		TCHAR text[1024];
		AfxLoadString(id, text, RTL_NUMBER_OF(text));
#if LANG_PO(FALSE, TRUE) // compiling for use with .PO files
		unsigned line = 0;
		_stscanf(text, _T("Merge.rc:%u"), &line);
		if (!TranslateString(line, s))
#endif
			s = text;
	}
	return s;
}

std::wstring CLanguageSelect::LoadDialogCaption(LPCTSTR lpDialogTemplateID) const
{
	std::wstring s;
	if (HINSTANCE hInst = AfxFindResourceHandle(lpDialogTemplateID, RT_DIALOG))
	{
		if (HRSRC hRsrc = FindResource(hInst, lpDialogTemplateID, RT_DIALOG))
		{
			if (LPCWSTR text = (LPCWSTR)LoadResource(hInst, hRsrc))
			{
				// Skip DLGTEMPLATE or DLGTEMPLATEEX
				text += text[1] == 0xFFFF ? 13 : 9;
				// Skip menu name string or ordinal
				if (*text == (const WCHAR)-1)
					text += 2; // WCHARs
				else
					while (*text++);
				// Skip class name string or ordinal
				if (*text == (const WCHAR)-1)
					text += 2; // WCHARs
				else
					while (*text++);
				// Caption string is ahead
#if LANG_PO(FALSE, TRUE) // compiling for use with .PO files
				unsigned line = 0;
				swscanf(text, L"Merge.rc:%u", &line);
				if (!TranslateString(line, s))
#endif
					s = text;
			}
		}
	}
	return s;
}

void CLanguageSelect::ReloadMenu() 
{
	if (m_idDocMenu)
	{
		// set the menu of the main frame window
		UINT idMenu = GetDocResId();
		CMergeApp *pApp = dynamic_cast<CMergeApp *> (AfxGetApp());
		CMainFrame * pMainFrame = dynamic_cast<CMainFrame *> ((CFrameWnd*)pApp->m_pMainWnd);
		HMENU hNewDefaultMenu = pMainFrame->NewDefaultMenu(idMenu);
		HMENU hNewMergeMenu = pMainFrame->NewMergeViewMenu();
		HMENU hNewDirMenu = pMainFrame->NewDirViewMenu();
		if (hNewDefaultMenu && hNewMergeMenu && hNewDirMenu)
		{
			CMenu* pOldDefaultMenu = CMenu::FromHandle(pMainFrame->m_hMenuDefault);
			CMenu* hOldMergeMenu = CMenu::FromHandle(pApp->m_pDiffTemplate->m_hMenuShared);
			CMenu* hOldDirMenu = CMenu::FromHandle(pApp->m_pDirTemplate->m_hMenuShared);

			// Note : for Windows98 compatibility, use FromHandle and not Attach/Detach
			CMenu * pNewDefaultMenu = CMenu::FromHandle(hNewDefaultMenu);
			CMenu * pNewMergeMenu = CMenu::FromHandle(hNewMergeMenu);
			CMenu * pNewDirMenu = CMenu::FromHandle(hNewDirMenu);
			
			CWnd *pFrame = CWnd::FromHandle(::GetWindow(pMainFrame->m_hWndMDIClient, GW_CHILD));
			while (pFrame)
			{
				if (pFrame->IsKindOf(RUNTIME_CLASS(CChildFrame)))
					((CChildFrame *)pFrame)->SetSharedMenu(hNewMergeMenu);
				else if (pFrame->IsKindOf(RUNTIME_CLASS(CDirFrame)))
					((CDirFrame *)pFrame)->SetSharedMenu(hNewDirMenu);
				pFrame = pFrame->GetNextWindow();
			}

			CFrameWnd *pActiveFrame = pMainFrame->GetActiveFrame();
			if (pActiveFrame)
			{
				if (pActiveFrame->IsKindOf(RUNTIME_CLASS(CChildFrame)))
					pMainFrame->MDISetMenu(pNewMergeMenu, NULL);
				else if (pActiveFrame->IsKindOf(RUNTIME_CLASS(CDirFrame)))
					pMainFrame->MDISetMenu(pNewDirMenu, NULL);
				else
					pMainFrame->MDISetMenu(pNewDefaultMenu, NULL);
			}
			else
				pMainFrame->MDISetMenu(pNewDefaultMenu, NULL);

			// Don't delete the old menu
			// There is a bug in BCMenu or in Windows98 : the new menu does not
			// appear correctly if we destroy the old one
//			if (pOldDefaultMenu)
//				pOldDefaultMenu->DestroyMenu();
//			if (pOldMergeMenu)
//				pOldMergeMenu->DestroyMenu();
//			if (pOldDirMenu)
//				pOldDirMenu->DestroyMenu();

			// m_hMenuDefault is used to redraw the main menu when we close a child frame
			// if this child frame had a different menu
			pMainFrame->m_hMenuDefault = hNewDefaultMenu;
			pApp->m_pDiffTemplate->m_hMenuShared = hNewMergeMenu;
			pApp->m_pDirTemplate->m_hMenuShared = hNewDirMenu;

			// force redrawing the menu bar
			pMainFrame->DrawMenuBar();  

		}
	}
}


UINT CLanguageSelect::GetDocResId()
{
	if (((CMDIFrameWnd*)AfxGetApp()->m_pMainWnd)->MDIGetActive())
		return m_idDocMenu;
	
	return m_idMainMenu;
}


void CLanguageSelect::UpdateDocTitle()
{
	CDocManager* pDocManager = AfxGetApp()->m_pDocManager;
	POSITION posTemplate = pDocManager->GetFirstDocTemplatePosition();
	ASSERT(posTemplate != NULL);

	while (posTemplate != NULL)
	{
		CDocTemplate* pTemplate = pDocManager->GetNextDocTemplate(posTemplate);
		
		ASSERT(pTemplate != NULL);
		
		POSITION pos = pTemplate->GetFirstDocPosition();
		CDocument* pDoc;
		
		while ( pos != NULL  )
		{
			pDoc = pTemplate->GetNextDoc(pos);
			pDoc->SetTitle(NULL);
			((CFrameWnd*)AfxGetApp()->m_pMainWnd)->OnUpdateFrameTitle(TRUE);
		}
	}
} 



void CLanguageSelect::OnOK() 
{
	UpdateData();
	int index = m_ctlLangList.GetCurSel();
	if (index<0) return;
	int i = m_ctlLangList.GetItemData(index);
	int lang = m_wLangIds[i];
	if ( lang != m_wCurLanguage )
	{
		SetLanguageOverride(lang);

		CMainFrame * pMainFrame = dynamic_cast<CMainFrame *> ((CFrameWnd*)AfxGetApp()->m_pMainWnd);
		pMainFrame->UpdateCodepageModule();

		// Update status bar inicator texts
		SetIndicators(pMainFrame->m_wndStatusBar, 0, 0);

		// Update the current menu
		if (m_bReloadMenu)
			ReloadMenu();
		
		// update the title text of the document
		if (m_bUpdateTitle)
			UpdateDocTitle();
	}
	
	EndDialog(IDOK);
}

void CLanguageSelect::OnDblclkLanguageList()
{
	OnOK();
}

BOOL CLanguageSelect::OnInitDialog()
{
	TranslateDialog(m_hWnd);
	CDialog::OnInitDialog();
	
	CMainFrame::SetMainIcon(this);

	// setup handler for resizing this dialog	
	m_constraint.InitializeCurrentSize(this);
	// configure how individual controls adjust when dialog resizes
	m_constraint.ConstrainItem(IDC_LANGUAGE_LIST, 0, 1, 0, 1); // grows right & down
	m_constraint.ConstrainItem(IDCANCEL, .6, 0, 1, 0); // slides down, floats right
	m_constraint.ConstrainItem(IDOK, .3, 0, 1, 0); // slides down, floats right
	m_constraint.SubclassWnd(); // install subclassing
	m_constraint.LoadPosition(_T("ResizeableDialogs"), _T("LanguageSelectDlg"), false); // persist size via registry

	GetMainFrame()->CenterToMainFrame(this);

	LoadAndDisplayLanguages();

	return TRUE;
}

/**
 * @brief Load languages available on disk, and display in list, and select current
 */
void CLanguageSelect::LoadAndDisplayLanguages()
{
	if (m_wLangIds.GetSize()<=0)
	{
		// get all available resource only Dlls
		//
		GetAvailLangs( m_wLangIds, m_DllFileNameAry );
		////
		
		// Add the language of this exe file to list at the
		// language select dialog
		//
		m_wLangIds.Add(wSourceLangId);  // Language Id of this english (US) application
		m_DllFileNameAry.Add("");	   // Dll Name - none
	}
		
// Fill the ComboBox
	CString Language;
	int i=0;
	for (i = 0; i < m_wLangIds.GetSize(); i++)
	{
		String Language = GetLanguageString(m_wLangIds[i]);
		if ( !Language.empty() )
		{
			int idx = m_ctlLangList.AddString(Language.c_str());
			m_ctlLangList.SetItemData(idx, i);
		}
	}
// Select the current language (if found)
	for (i=0; i<m_ctlLangList.GetCount(); ++i)
	{
		if (m_wCurLanguage == m_wLangIds[m_ctlLangList.GetItemData(i)])
		{
			m_ctlLangList.SetCurSel(i);
			break;
		}
	}
}


int CLanguageSelect::GetLanguageArrayIndex( WORD LangId )
{
	for ( int i = 0; lang_map[i].m_LangId != 0; i++)
		if ( lang_map[i].m_LangId == LangId)
			return i;
	
	return -1;
}
	
String CLanguageSelect::GetLanguageString( WORD LangId )
	{
	int idx = GetLanguageArrayIndex(LangId);
	if (idx == -1) return _T("");

	// Localized name
	String Language = theApp.LoadString(lang_map[idx].m_IdName);
	// Append native name
	Language += _T(" - ");
	Language += GetNativeLanguageNameString(idx);
	return Language;
	}

CString CLanguageSelect::GetNativeLanguageNameString( int idx )
	{
	CString Language(_T(""));
	// Display the native name (from the array in this file) if it fits into current codepage
	// Otherwise, take the name from the RC file (which will be the name from the English RC
	// file, as none of the other RC files have language name entries, and the names in the
	// English RC file are all ASCII, so they fit into any codepage)
	//
	// Note: Even in Unicode build, we still do this test of conversion to current codepage
	// because if the name fits in the current codepage, then we're sure they have glyphs
	// otherwise, they might not have glyphs in their current font, so it might be illegible
	LPCWSTR name = lang_map[idx].m_NativeName;
	if (name[0])
	{
		int codepage = GetACP();
		int flags = 0;
		char cbuffer[256];

		BOOL defaulted = FALSE;
		int nbytes = WideCharToMultiByte(codepage, flags, name, wcslen(name), cbuffer, sizeof(cbuffer), 0, &defaulted);
		if (nbytes && !defaulted)
		{
			cbuffer[nbytes] = 0;
#ifdef _UNICODE
			Language = name;
#else
			Language = cbuffer;
#endif
		}
	}

	if (Language.IsEmpty())
		Language = lang_map[idx].m_AsciiName;
	
	return Language;
}

/**
 * @brief Find DLL entry in lang_map for language for specified locale
 */
static WORD
GetLangFromLocale(LCID lcid)
{
	TCHAR buff[8] = {0};
	if (GetLocaleInfo(lcid, LOCALE_IDEFAULTLANGUAGE, buff, countof(buff)))
	{
		int langID = 0;
		if ((1 == _stscanf(buff, _T("%4x"), &langID)) && langID)
			return (WORD)langID;
	}
	return (WORD)-1;
}

void
CLanguageSelect::InitializeLanguage()
{
	int iLangId = AfxGetApp()->GetProfileInt( LANGUAGE_SECTION, COUNTRY_ENTRY, (INT) -1 );
	if ( iLangId != -1 )
	{
		// User has set a language override
		SetLanguageOverride((WORD)iLangId);
	}
	else
	{
		// User has not specified a language
		// so look in thread locale, user locale, and then system locale for
		// a language that WinMerge supports

		WORD Lang1 = GetLangFromLocale(GetThreadLocale());
		if (Lang1 != (WORD)-1)
		{
			CString dll = GetDllName(Lang1);
			if (!dll.IsEmpty() && LoadResourceDLL(dll))
			{
				SetLanguage(Lang1);
				return;
			}
		}
		WORD Lang2 = GetLangFromLocale(LOCALE_USER_DEFAULT);
		if (Lang2 != (WORD)-1 && Lang2 != Lang1)
		{
			CString dll = GetDllName(Lang2);
			if (!dll.IsEmpty() && LoadResourceDLL(dll))
			{
				SetLanguage(Lang2);
				return;
			}
		}
		WORD Lang3 = GetLangFromLocale(LOCALE_SYSTEM_DEFAULT);
		if (Lang3 != (WORD)-1 && Lang3 != Lang2 && Lang3 != Lang1)
		{
			CString dll = GetDllName(Lang3);
			if (!dll.IsEmpty() && LoadResourceDLL(dll))
			{
				SetLanguage(Lang3);
				return;
			}
		}
	}
}

