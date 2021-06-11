#pragma once
#include <ctype.h>
struct Lang_Code
{
    char *Code;
    wchar_t *Name;
};

#define LANG_CODE_TABLE {{"de", L"Deutsch"}, \
{"en", L"English"}, \
{"es", L"español"}, \
{"es_419", L"español (Latinoamérica)"}, \
{"fr", L"français"}, \
{"hr", L"hrvatski"}, \
{"it", L"italiano"}, \
{"nl", L"Nederlands"}, \
{"pl", L"polski"}, \
{"pt_BR", L"português (Brasil)"}, \
{"pt_PT", L"português (Portugal)"}, \
{"vi", L"Tiếng Việt"}, \
{"tr", L"Türkçe"}, \
{"ru", L"русский"}, \
{"ar", L"العربية"}, \
{"th", L"ไทย"}, \
{"ko", L"한국어"}, \
{"zh_CN", L"中文 (简体)"}, \
{"zh_TW", L"中文 (繁體)"}, \
{"ja", L"日本語"}, \
{"ach", L"Acoli"}, \
{"af", L"Afrikaans"}, \
{"ak", L"Akan"}, \
{"az", L"azərbaycan"}, \
{"ban", L"Balinese"}, \
{"su", L"Basa Sunda"}, \
{"ceb", L"Binisaya"}, \
{"xx_bork", L"Bork, bork, bork!"}, \
{"bs", L"bosanski"}, \
{"br", L"brezhoneg"}, \
{"ca", L"català"}, \
{"cs", L"čeština"}, \
{"sn", L"chiShona"}, \
{"co", L"Corsican"}, \
{"cy", L"Cymraeg"}, \
{"da", L"dansk"}, \
{"yo", L"Èdè Yorùbá"}, \
{"et", L"eesti"}, \
{"xx_elmer", L"Elmer Fudd"}, \
{"eo", L"esperanto"}, \
{"eu", L"euskara"}, \
{"ee", L"Eʋegbe"}, \
{"tl", L"Filipino"}, \
{"fil", L"Filipino"}, \
{"fo", L"føroyskt"}, \
{"fy", L"Frysk"}, \
{"gaa", L"Ga"}, \
{"ga", L"Gaeilge"}, \
{"gd", L"Gàidhlig"}, \
{"gl", L"galego"}, \
{"gn", L"Guarani"}, \
{"xx_hacker", L"Hacker"}, \
{"ht", L"Haitian Creole"}, \
{"ha", L"Hausa"}, \
{"haw", L"ʻŌlelo Hawaiʻi"}, \
{"bem", L"Ichibemba"}, \
{"ig", L"Igbo"}, \
{"rn", L"Ikirundi"}, \
{"id", L"Indonesia"}, \
{"ia", L"interlingua"}, \
{"xh", L"isiXhosa"}, \
{"zu", L"isiZulu"}, \
{"is", L"íslenska"}, \
{"jw", L"Jawa"}, \
{"rw", L"Kinyarwanda"}, \
{"sw", L"Kiswahili"}, \
{"tlh", L"Klingon"}, \
{"kg", L"Kongo"}, \
{"mfe", L"kreol morisien"}, \
{"kri", L"Krio (Sierra Leone)"}, \
{"la", L"Latin"}, \
{"lv", L"latviešu"}, \
{"to", L"lea fakatonga"}, \
{"lt", L"lietuvių"}, \
{"ln", L"lingála"}, \
{"loz", L"Lozi"}, \
{"lua", L"Luba_Lulua"}, \
{"lg", L"Luganda"}, \
{"hu", L"magyar"}, \
{"mg", L"Malagasy"}, \
{"mt", L"Malti"}, \
{"ms", L"Melayu"}, \
{"pcm", L"Nigerian Pidgin"}, \
{"no", L"norsk"}, \
{"nn", L"norsk nynorsk"}, \
{"nso", L"Northern Sotho"}, \
{"ny", L"Nyanja"}, \
{"uz", L"o‘zbek"}, \
{"oc", L"Occitan"}, \
{"om", L"Oromoo"}, \
{"xx_pirate", L"Pirate"}, \
{"ro", L"română"}, \
{"rm", L"rumantsch"}, \
{"qu", L"Runasimi"}, \
{"nyn", L"Runyankore"}, \
{"crs", L"Seychellois Creole"}, \
{"sq", L"shqip"}, \
{"sk", L"slovenčina"}, \
{"sl", L"slovenščina"}, \
{"so", L"Soomaali"}, \
{"st", L"Southern Sotho"}, \
{"sr_ME", L"srpski (Crna Gora)"}, \
{"sr_Latn", L"srpski (latinica)"}, \
{"fi", L"suomi"}, \
{"sv", L"svenska"}, \
{"mi", L"te reo Māori"}, \
{"tn", L"Tswana"}, \
{"tum", L"Tumbuka"}, \
{"tk", L"türkmen dili"}, \
{"tw", L"Twi"}, \
{"wo", L"Wolof"}, \
{"el", L"Ελληνικά"}, \
{"be", L"беларуская"}, \
{"bg", L"български"}, \
{"ky", L"кыргызча"}, \
{"kk", L"қазақ тілі"}, \
{"mk", L"македонски"}, \
{"mn", L"монгол"}, \
{"sr", L"српски"}, \
{"tt", L"татар"}, \
{"tg", L"тоҷикӣ"}, \
{"uk", L"українська"}, \
{"ka", L"ქართული"}, \
{"hy", L"հայերեն"}, \
{"yi", L"ייִדיש"}, \
{"iw", L"עברית"}, \
{"ug", L"ئۇيغۇرچە"}, \
{"ur", L"اردو"}, \
{"ps", L"پښتو"}, \
{"sd", L"سنڌي"}, \
{"fa", L"فارسی"}, \
{"ckb", L"کوردیی ناوەندی"}, \
{"ti", L"ትግር"}, \
{"am", L"አማርኛ"}, \
{"ne", L"नेपाली"}, \
{"mr", L"मराठी"}, \
{"hi", L"हिन्दी"}, \
{"bn", L"বাংলা"}, \
{"pa", L"ਪੰਜਾਬੀ"}, \
{"gu", L"ગુજરાતી"}, \
{"or", L"ଓଡ଼ିଆ"}, \
{"ta", L"தமிழ்"}, \
{"te", L"తెలుగు"}, \
{"kn", L"ಕನ್ನಡ"}, \
{"ml", L"മലയാളം"}, \
{"si", L"සිංහල"}, \
{"lo", L"ລາວ"}, \
{"my", L"မြန်မာ"}, \
{"km", L"ខ្មែរ"}, \
{"chr", L"ᏣᎳᎩ"}}

int
strncasecmp(const char *s1, const char *s2, size_t nch)
{
    size_t          ii;
    int             res = -1;

    if (!s1) {
        if (!s2)
            return 0;
        return (-1);
    }
    if (!s2)
        return (1);

    for (ii = 0; (ii < nch) && *s1 && *s2; ii++, s1++, s2++) {
        res = (int) (tolower(*s1) - tolower(*s2));
        if (res != 0)
            break;
    }

    if (ii == nch) {
        s1--;
        s2--;
    }

    if (!*s1) {
        if (!*s2)
            return 0;
        return (-1);
    }
    if (!*s2)
        return (1);

    return (res);
}

int
strcasecmp(const char *s1, const char *s2)
{
    return strncasecmp(s1, s2, 1000000);
}

static const wchar_t* GetLangNameByCode(const char *langCode)
{
    static struct Lang_Code codeTable[] = LANG_CODE_TABLE;
    for (size_t i = 0; i < sizeof(codeTable) / sizeof(struct Lang_Code); i++)
    {
        if(!strcasecmp(langCode, codeTable[i].Code)){
            return codeTable[i].Name;
        }
    }
    return nullptr;
}