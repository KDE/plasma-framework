/* This file is part of the KDE libraries
    Copyright (C) 2012 Giorgos Tsiapaliwkas <terietor@gmail.com>
    Copyright (C) 2012 Antonis Tsiapaliokas <kok3rs@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef LOCALE_H
#define LOCALE_H

//Qt
#include <QObject>
#include <QTime>
#include <QDate>

//KDE
#include <KLocale>

class QString;
class QDate;
class QTime;
class QDateTime;

/**
 * \file klocale.h
 */

/**
  *
  * KLocale provides support for country specific stuff like
  * the national language.
  *
  * KLocale supports translating, as well as specifying the format
  * for numbers, currency, time, and date.
  *
  * Use KGlobal::locale() to get pointer to the global KLocale object,
  * containing the applications current locale settings.
  *
  * For example, to format the date May 17, 1995 in the current locale, use:
  *
  * \code
  *   QString date = KGlobal::locale()->formatDate(QDate(1995,5,17));
  * \endcode
  *
  * @author Stephan Kulow <coolo@kde.org>, Preston Brown <pbrown@kde.org>,
  * Hans Petter Bieker <bieker@kde.org>, Lukas Tinkl <lukas.tinkl@suse.cz>
  * @short class for supporting locale settings and national language
  */
class Locale : public QObject
{
Q_OBJECT

//enuns
Q_ENUMS(BinarySizeUnits)
Q_ENUMS(BinaryUnitDialect)
Q_ENUMS(CalendarSystem)
Q_ENUMS(DateFormat)
Q_ENUMS(DateTimeComponent)
Q_ENUMS(DateTimeComponentFormat)
Q_ENUMS(DateTimeFormatOption )
Q_ENUMS(DigitSet)
Q_ENUMS(MeasureSystem)
Q_ENUMS(ReadDateFlags)
Q_ENUMS(SignPosition)
Q_ENUMS(TimeFormatOption)
Q_ENUMS(TimeProcessingOption)
Q_ENUMS(WeekNumberSystem)

//properties
Q_PROPERTY(BinaryUnitDialect binaryUnitDialect READ binaryUnitDialect WRITE setBinaryUnitDialect NOTIFY binaryUnitDialectChanged)
Q_PROPERTY(Locale::CalendarSystem calendarSystem READ calendarSystem WRITE setCalendarSystem NOTIFY calendarSystemChanged)
Q_PROPERTY(QString country READ country CONSTANT) //read-only
Q_PROPERTY(QString countryDivisionCode READ countryDivisionCode WRITE setCountryDivisionCode NOTIFY countryDivisionCodeChanged)
Q_PROPERTY(QString currencyCode READ currencyCode WRITE setCurrencyCode NOTIFY currencyCodeChanged)
Q_PROPERTY(QString currencySymbol READ currencySymbol WRITE setCurrencySymbol NOTIFY currencySymbolChanged)
Q_PROPERTY(QString dateFormat READ dateFormat WRITE setDateFormat NOTIFY dateFormatChanged)
Q_PROPERTY(QString dateFormatShort READ dateFormatShort WRITE setDateFormat NOTIFY dateFormatShortChanged)
Q_PROPERTY(QString timeFormat READ timeFormat WRITE setTimeFormat NOTIFY timeFormatChanged)
Q_PROPERTY(bool dateMonthNamePossessive READ dateMonthNamePossessive WRITE setDateMonthNamePossessive NOTIFY dateMonthNamePossessiveChanged)
Q_PROPERTY(DigitSet dateTimeDigitSet READ dateTimeDigitSet WRITE setDateTimeDigitSet NOTIFY dateTimeDigitSetChanged)
Q_PROPERTY(int decimalPlaces READ decimalPlaces WRITE setDecimalPlaces NOTIFY decimalPlacesChanged)
Q_PROPERTY(QString decimalSymbol READ decimalSymbol WRITE setDecimalSymbol NOTIFY decimalSymbolChanged)
Q_PROPERTY(DigitSet digitSet READ digitSet WRITE setDigitSet NOTIFY digitSetChanged)
Q_PROPERTY(QString language READ language CONSTANT) //read-only
Q_PROPERTY(MeasureSystem measureSystem READ measureSystem WRITE setMeasureSystem NOTIFY measureSystemChanged)
Q_PROPERTY(int monetaryDecimalPlaces READ monetaryDecimalPlaces WRITE setMonetaryDecimalPlaces NOTIFY monetaryDecimalPlacesChanged)
Q_PROPERTY(QString monetaryDecimalSymbol READ monetaryDecimalSymbol WRITE setMonetaryDecimalSymbol NOTIFY monetaryDecimalSymbolChanged)
Q_PROPERTY(DigitSet monetaryDigitSet READ monetaryDigitSet WRITE setMonetaryDigitSet NOTIFY monetaryDigitSetChanged)
Q_PROPERTY(QString monetaryThousandsSeparator READ monetaryThousandsSeparator WRITE setMonetaryThousandsSeparator NOTIFY monetaryThousandsSeparatorChanged)
Q_PROPERTY(SignPosition negativeMonetarySignPosition READ negativeMonetarySignPosition WRITE setNegativeMonetarySignPosition NOTIFY negativeMonetarySignPositionChanged)
Q_PROPERTY(bool negativePrefixCurrencySymbol READ negativePrefixCurrencySymbol WRITE setNegativePrefixCurrencySymbol NOTIFY negativePrefixCurrencySymbolChanged)
Q_PROPERTY(QString negativeSign READ negativeSign WRITE setNegativeSign NOTIFY negativeSignChanged)
Q_PROPERTY(int pageSize READ pageSize WRITE setPageSize NOTIFY pageSizeChanged)
Q_PROPERTY(SignPosition positiveMonetarySignPosition READ positiveMonetarySignPosition WRITE setPositiveMonetarySignPosition NOTIFY positiveMonetarySignPositionChanged)
Q_PROPERTY(bool positivePrefixCurrencySymbol READ positivePrefixCurrencySymbol WRITE setPositivePrefixCurrencySymbol NOTIFY positivePrefixCurrencySymbolChanged)
Q_PROPERTY(QString positiveSign READ positiveSign WRITE setPositiveSign NOTIFY positiveSignChanged)
Q_PROPERTY(QString thousandsSeparator READ thousandsSeparator WRITE setThousandsSeparator NOTIFY thousandsSeparatorChanged)
Q_PROPERTY(int weekDayOfPray READ weekDayOfPray WRITE setWeekDayOfPray NOTIFY weekDayOfPrayChanged)
Q_PROPERTY(Locale::WeekNumberSystem weekNumberSystem READ weekNumberSystem WRITE setWeekNumberSystem NOTIFY WeekNumberSystemChanged)
Q_PROPERTY(int weekStartDay READ weekStartDay WRITE setWeekStartDay NOTIFY weekStartDayChanged)
Q_PROPERTY(int workingWeekEndDay READ workingWeekEndDay WRITE setWorkingWeekEndDay NOTIFY workingWeekEndDayChanged)
Q_PROPERTY(int workingWeekStartDay READ workingWeekStartDay WRITE setWorkingWeekStartDay NOTIFY workingWeekStartDayChanged)
Q_PROPERTY(bool use12Clock READ use12Clock CONSTANT)
Q_PROPERTY(QString defaultLanguage READ defaultLanguage CONSTANT)//read-only
Q_PROPERTY(QString defaultCountry READ defaultCountry CONSTANT)//read-only
Q_PROPERTY(QString defaultCurrencyCode READ defaultCurrencyCode CONSTANT)//read-only
Q_PROPERTY(bool useTranscript READ useTranscript CONSTANT) //read-only
Q_PROPERTY(int fileEncodingMib READ fileEncodingMib CONSTANT) //read-only
Q_PROPERTY(QStringList languageList READ languageList CONSTANT) //read-only
Q_PROPERTY(QStringList currencyCodeList READ currencyCodeList CONSTANT) //read-only
Q_PROPERTY(QStringList allLanguagesList READ allLanguagesList CONSTANT) //read-only
Q_PROPERTY(QStringList installedLanguages READ installedLanguages CONSTANT) //read-only
Q_PROPERTY(QStringList allCountriesList READ allCountriesList CONSTANT) //read-only
Q_PROPERTY(QList<int> allDigitSetsList READ allDigitSetsList CONSTANT) //read-only

public:
    /**
     * ctor
     */
    Locale(QObject *parent = 0);

    /**
     * Various positions for where to place the positive or negative
     * sign when they are related to a monetary value.
     */
    enum SignPosition {
        /**
         * Put parantheses around the quantity, e.g. "$ (217)"
         */
        ParensAround = 0,
        /**
         * Prefix the quantity with the sign, e.g. "$ -217"
         */
        BeforeQuantityMoney = 1,
        /**
         * Suffix the quanitity with the sign, e.g. "$ 217-"
         */
        AfterQuantityMoney = 2,
        /**
         * Prefix the currency symbol with the sign, e.g. "-$ 217"
         */
        BeforeMoney = 3,
        /**
         * Suffix the currency symbol with the sign, e.g. "$- 217"
         */
        AfterMoney = 4
    };

    /**
     *
     * The set of digit characters used to display and enter numbers.
     */
    enum DigitSet {
        ArabicDigits,             /**< 0123456789 (European and some Asian
                                       languages and western Arabic dialects) */
        ArabicIndicDigits,        /**< ٠١٢٣٤٥٦٧٨٩ (eastern Arabic dialects) */
        EasternArabicIndicDigits, /**< ۰۱۲۳۴۵۶۷۸۹ (Persian and Urdu) */
        DevenagariDigits,         /**< ०१२३४५६७८९ (Hindi) */
        BengaliDigits,            /**< ০১২৩৪৫৬৭৮৯ (Bengali and  Assamese) */
        GujaratiDigits,           /**< ૦૧૨૩૪૫૬૭૮૯ (Gujarati) */
        GurmukhiDigits,           /**< ੦੧੨੩੪੫੬੭੮੯ (Punjabi) */
        KannadaDigits,            /**< ೦೧೨೩೪೫೬೭೮೯ (Kannada) */
        KhmerDigits,              /**< ០១២៣៤៥៦៧៨៩ (Khmer) */
        MalayalamDigits,          /**< ൦൧൨൩൪൫൬൭൮൯ (Malayalam) */
        OriyaDigits,              /**< ୦୧୨୩୪୫୬୭୮୯ (Oriya) */
        TamilDigits,              /**< ௦௧௨௩௪௫௬௭௮ (Tamil) */
        TeluguDigits,             /**< ౦౧౨౩౪౫౬౭౯ (Telugu) */
        ThaiDigits                /**< ๐๑๒๓๔๕๖๗๘๙ (Thai) */
    // The following Decimal Digit Sets are defined in Unicode but the associated
    // languages are not yet translated in KDE, so are not yet enabled.
    // The script names are taken from the Unicode standard, the associated
    // languages from Wikipedia.
    //  BalineseDigits,           /**< ᭐᭑᭒᭓᭔᭕᭖᭗᭘᭙ (Balinese) */
    //  ChamDigits,               /**< ꩐꩑꩒꩓꩔꩕꩖꩗꩘꩙ (Cham) */
    //  JavaneseDigits,           /**< ꧐꧑꧒꧓꧔꧕꧖꧗꧘꧙ (Javanese) */
    //  KayahLiDigits,            /**< ꤀꤁꤂꤃꤄꤅꤆꤇꤈꤉ (Kayah) */
    //  LaoDigits,                /**< ໐໑໒໓໔໕໖໗໘໙ (Lao) */
    //  LepchaDigits,             /**< ᱀᱁᱂᱃᱄᱅᱆᱇᱈᱉ (Lepcha) */
    //  LimbuDigits,              /**< ᥆᥇᥈᥉᥊᥋᥌᥍᥎᥏ (Limbu) */
    //  MeeteiMayekDigits,        /**< ꯰꯱꯲꯳꯴꯵꯶꯷꯸꯹ (Meitei) */
    //  MongolianDigits,          /**< ᠐᠑᠒᠓᠔᠕᠖᠗᠘᠙ (Mongolian) */
    //  MyanmarDigits,            /**< ၀၁၂၃၄၅၆၇၈၉ (Myanmar/Burmese ) */
    //  MyanmarShanDigits,        /**< ႐႑႒႓႔႕႖႗႘႙ (Shan) */
    //  NewTaiLueDigits,          /**< ᧐᧑᧒᧓᧔᧕᧖᧗᧘᧙ (Tai Lü) */
    //  NKoDigits,                /**< ߀߁߂߃߄߅߆߇߈߉ (Mande and N'Ko) */
    //  OlChikiDigits,            /**< ᱐᱑᱒᱓᱔᱕᱖᱗᱘᱙ (Santali) */
    //  OsmanyaDigits,            /**< ҠҡҢңҤҥҦҧҨҩ (Somali) */
    //  SaurashtraDigits,         /**< ꣐꣑꣒꣓꣔꣕꣖꣗꣘꣙ (Saurashtra) */
    //  SundaneseDigits,          /**< ᮰᮱᮲᮳᮴᮵᮶᮷᮸᮹ (Sundanese) */
    //  TaiThamDigits,            /**< ᪐᪑᪒᪓᪔᪕᪖᪗᪘᪙ (Tai Lü) */
    //  TibetanDigits,            /**< ༠༡༢༣༤༥༦༧༨༩ (Tibetan) */
    //  VaiDigits,                /**< ꘠꘡꘢꘣꘤꘥꘦꘧꘨꘩ (Vai) */
    };

    /**
     *
     * Convert a digit set identifier to a human readable, localized name.
     *
     * @param digitSet the digit set identifier
     * @param withDigits whether to add the digits themselves to the name
     *
     * @return the human readable and localized name of the digit set
     *
     * @see DigitSet
     */
    QString digitSetToName(DigitSet digitSet, bool withDigits = false) const;

    /**
     *
     * Provides list of all known digit set identifiers.
     *
     * @return list of all digit set identifiers
     * @see DigitSet
     * @see digitSetToName
     */
    QList<int> allDigitSetsList() const;

    /**
     * Returns what a decimal point should look like ("." or "," etc.)
     * according to the current locale or user settings.
     *
     * @return The decimal symbol used by locale.
     */
    QString decimalSymbol() const;

    /**
     * Returns what the thousands separator should look
     * like ("," or "." etc.)
     * according to the current locale or user settings.
     *
     * @return The thousands separator used by locale.
     */
    QString thousandsSeparator() const;

    /**
     *
     * Returns the identifier of the digit set used to display numbers.
     *
     * @return the digit set identifier
     * @see DigitSet
     * @see digitSetToName
     */
    DigitSet digitSet() const;

    /**
     *
     * Returns the ISO 4217 Currency Code for the current locale
     *
     * @return The default ISO Currency Code used by locale.
    */
    QString currencyCode() const;

    /**
     * Returns what the symbol denoting currency in the current locale
     * as as defined by user settings should look like.
     *
     * @return The default currency symbol used by locale.
     */
    QString currencySymbol() const;

    /**
     * Returns what a decimal point should look like ("." or "," etc.)
     * for monetary values, according to the current locale or user
     * settings.
     *
     * @return The monetary decimal symbol used by locale.
     */
    QString monetaryDecimalSymbol() const;

    /**
     * Returns what a thousands separator for monetary values should
     * look like ("," or " " etc.) according to the current locale or
     * user settings.
     *
     * @return The monetary thousands separator used by locale.
     */
    QString monetaryThousandsSeparator() const;

    /**
     * Returns what a positive sign should look like ("+", " ", etc.)
     * according to the current locale or user settings.
     *
     * @return The positive sign used by locale.
     */
    QString positiveSign() const;

    /**
     * Returns what a negative sign should look like ("-", etc.)
     * according to the current locale or user settings.
     *
     * @return The negative sign used by locale.
     */
    QString negativeSign() const;

    /**
     *
     * The number of decimal places to include in numeric values (usually 2).
     *
     * @return Default number of numeric decimal places used by locale.
     */
    int decimalPlaces() const;

    /**
     *
     * The number of decimal places to include in monetary values (usually 2).
     *
     * @return Default number of monetary decimal places used by locale.
     */
    int monetaryDecimalPlaces() const;

    /**
     * If and only if the currency symbol precedes a positive value,
     * this will be true.
     *
     * @return Where to print the currency symbol for positive numbers.
     */
    bool positivePrefixCurrencySymbol() const;

    /**
     * If and only if the currency symbol precedes a negative value,
     * this will be true.
     *
     * @return True if the currency symbol precedes negative numbers.
     */
    bool negativePrefixCurrencySymbol() const;

    /**
     * Returns the position of a positive sign in relation to a
     * monetary value.
     *
     * @return Where/how to print the positive sign.
     * @see SignPosition
     */
    SignPosition positiveMonetarySignPosition() const;

    /**
     * Denotes where to place a negative sign in relation to a
     * monetary value.
     *
     * @return Where/how to print the negative sign.
     * @see SignPosition
     */
    SignPosition negativeMonetarySignPosition() const;

    /**
     *
     * Retuns the digit set used to display monetary values.
     *
     * @return the digit set identifier
     * @see DigitSet
     * @see digitSetToName
     */
    DigitSet monetaryDigitSet() const;

    /**
     * Given a double, converts that to a numeric string containing
     * the localized monetary equivalent.
     *
     * e.g. given 123456, return "$ 123,456.00".
     *
     * If precision isn't specified or is < 0, then the default monetaryDecimalPlaces() is used.
     *
     * @param num The number we want to format
     * @param currency The currency symbol you want.
     * @param precision Number of decimal places displayed
     *
     * @return The number of money as a localized string
     * @see monetaryDecimalPlaces()
     */
    Q_INVOKABLE QString formatMoney(double num, const QString &currency = QString(), int precision = -1) const;

    /**
     * Given a string representing a number, converts that to a numeric
     * string containing the localized numeric equivalent.
     *
     * e.g. given 123456.78F, return "123,456.78" (for some European country).
     *
     * If precision isn't specified or is < 0, then the default decimalPlaces() is used.
     *
     * @param numStr The number to format, as a string.
     * @param round Round fractional digits. (default true)
     * @param precision Number of fractional digits used for rounding. Unused if round=false.
     *
     * @return The number as a localized string
     */
    Q_INVOKABLE QString formatNumber(const QString &numStr, bool round = true, int precision = -1) const;

    /**
     * Given an integer, converts that to a numeric string containing
     * the localized numeric equivalent.
     *
     * e.g. given 123456L, return "123,456" (for some European country).
     *
     * @param num The number to convert
     *
     * @return The number as a localized string
     */
    Q_INVOKABLE QString formatLong(long num) const;

    /**
     * These binary units are used in KDE by the formatByteSize()
     * functions.
     *
     * NOTE: There are several different units standards:
     * 1) SI  (i.e. metric), powers-of-10.
     * 2) IEC, powers-of-2, with specific units KiB, MiB, etc.
     * 3) JEDEC, powers-of-2, used for solid state memory sizing which
     *    is why you see flash cards labels as e.g. 4GB.  These (ab)use
     *    the metric units.  Although JEDEC only defines KB, MB, GB, if
     *    JEDEC is selected all units will be powers-of-2 with metric
     *    prefixes for clarity in the event of sizes larger than 1024 GB.
     *
     * Although 3 different dialects are possible this enum only uses
     * metric names since adding all 3 different names of essentially the same
     * unit would be pointless.  Use BinaryUnitDialect to control the exact
     * units returned.
     *
     * @see binaryUnitDialect
     */
    enum BinarySizeUnits {
        /// Auto-choose a unit such that the result is in the range [0, 1000 or 1024)
        DefaultBinaryUnits = 1000,

        // The first real unit must be 0 for the current implementation!
        UnitByte = 0,      ///<  B         1 byte
        UnitKiloByte,  ///<  KiB/KB/kB 1024/1000 bytes.
        UnitMegaByte,  ///<  MiB/MB/MB 2^20/10^06 bytes.
        UnitGigaByte,  ///<  GiB/GB/GB 2^30/10^09 bytes.
        UnitTeraByte,  ///<  TiB/TB/TB 2^40/10^12 bytes.
        UnitPetaByte,  ///<  PiB/PB/PB 2^50/10^15 bytes.
        UnitExaByte,   ///<  EiB/EB/EB 2^60/10^18 bytes.
        UnitZettaByte, ///<  ZiB/ZB/ZB 2^70/10^21 bytes.
        UnitYottaByte, ///<  YiB/YB/YB 2^80/10^24 bytes.
        UnitLastUnit = UnitYottaByte
    };

    /**
     * This enum chooses what dialect is used for binary units.
     *
     * Note: Although JEDEC abuses the metric prefixes and can therefore be
     * confusing, it has been used to describe *memory* sizes for quite some time
     * and programs should therefore use either Default, JEDEC, or IEC 60027-2
     * for memory sizes.
     *
     * On the other hand network transmission rates are typically in metric so
     * Default, Metric, or IEC (which is unambiguous) should be chosen.
     *
     * Normally choosing DefaultBinaryUnits is the best option as that uses
     * the user's selection for units.
     *
     * @see binaryUnitDialect
     * @see setBinaryUnitDialect
     */
    enum BinaryUnitDialect {
        DefaultBinaryDialect = 1000, ///< Used if no specific preference
        IECBinaryDialect = 0,          ///< KDE Default, KiB, MiB, etc. 2^(10*n)
        JEDECBinaryDialect,        ///< KDE 3.5 default, KB, MB, etc. 2^(10*n)
        MetricBinaryDialect,       ///< SI Units, kB, MB, etc. 10^(3*n)
        LastBinaryDialect = MetricBinaryDialect
    };

    /**
     * Converts @p size from bytes to the string representation using the
     * user's default binary unit dialect.  The default unit dialect is
     * IEC 60027-2.
     *
     * Example:
     * formatByteSize(1024) returns "1.0 KiB" by default.
     *
     * @param  size  size in bytes
     * @return converted size as a string - e.g. 123.4 KiB , 12.0 MiB
     * @see BinaryUnitDialect
     * @todo KDE 5: Remove in favor of overload added in KDE 4.4.
     */
    Q_INVOKABLE QString formatByteSize(double size) const;

    /**
     *
     * Converts @p size from bytes to the appropriate string representation
     * using the binary unit dialect @p dialect and the specific units @p specificUnit.
     *
     * Example:
     * formatByteSize(1000, unit, Locale::BinaryUnitKilo) returns:
     *   for Locale::MetricBinaryUnits, "1.0 kB",
     *   for Locale::IECBinaryUnits,    "0.9 KiB",
     *   for Locale::JEDECBinaryUnits,  "0.9 KB".
     *
     * @param size size in bytes
     * @param precision number of places after the decimal point to use.  KDE uses
     *        1 by default so when in doubt use 1.
     * @param dialect binary unit standard to use.  Use DefaultBinaryUnits to
     *        use the localized user selection unless you need to use a specific
     *        unit type (such as displaying a flash memory size in JEDEC).
     * @param specificUnit specific unit size to use in result.  Use
     *        DefaultBinarySize to automatically select a unit that will return
     *        a sanely-sized number.
     * @return converted size as a translated string including the units.
     *         E.g. "1.23 KiB", "2 GB" (JEDEC), "4.2 kB" (Metric).
     * @see BinaryUnitDialect
     */
    QString formatByteSize(double size, int precision,
                           BinaryUnitDialect dialect = Locale::DefaultBinaryDialect,
                           BinarySizeUnits specificUnit = Locale::DefaultBinaryUnits) const;

    /**
     * Returns the user's configured binary unit dialect.
     * e.g. if MetricBinaryDialect is returned then the values
     * configured for how much a set of bytes are worth would
     * be 10^(3*n) and KB (1000 bytes == 1 KB), in this case.
     *
     * Will never return DefaultBinaryDialect.
     *
     * @return User's configured binary unit dialect
     * @see BinaryUnitDialect
     */
    BinaryUnitDialect binaryUnitDialect() const;

    /**
     * Sets @p newDialect to be the default dialect for this locale (and only
     * this locale).  Newly created KLocale objects will continue to default
     * to the user's choice.
     *
     * @param newDialect the new dialect to set as default for this locale object.
     */
    void setBinaryUnitDialect(BinaryUnitDialect newDialect);

    /**
     * Given a number of milliseconds, converts that to a string containing
     * the localized equivalent
     *
     * e.g. given formatDuration(60000), returns "1.0 minutes"
     *
     * @param mSec Time duration in milliseconds
     * @return converted duration as a string - e.g. "5.5 seconds" "23.0 minutes"
     */
    Q_INVOKABLE QString formatDuration(unsigned long mSec) const;

    /**
     * Given a number of milliseconds, converts that to a pretty string containing
     * the localized equivalent.
     *
     * e.g. given prettyFormatDuration(60001) returns "1 minute"
     *      given prettyFormatDuration(62005) returns "1 minute and 2 seconds"
     *      given prettyFormatDuration(90060000) returns "1 day and 1 hour"
     *
     * @param mSec Time duration in milliseconds
     * @return converted duration as a string.
     *         Units not interesting to the user, for example seconds or minutes when the first
     *         unit is day, are not returned because they are irrelevant. The same applies for
     *         seconds when the first unit is hour.
     */
    Q_INVOKABLE QString prettyFormatDuration(unsigned long mSec) const;

    /**
     *
     * Available Calendar Systems
     *
     * @see setCalendarSystem()
     * @see calendarSystem()
     */
    enum CalendarSystem {
        QDateCalendar = 1, /**< KDE Default, hybrid of Gregorian and Julian as used by QDate */
        //BahaiCalendar = 2, /**< Baha'i Calendar */
        //BuddhistLunarCalendar = 3, /**< Buddhist Lunar Calendar*/
        //ChineseCalendar = 4, /**< Chinese Calendar */
        CopticCalendar = 5, /**< Coptic Calendar as used Coptic Church and some parts of Egypt */
        EthiopianCalendar = 6, /**< Ethiopian Calendar, aka Ethiopic Calendar */
        //EthiopianAmeteAlemCalendar = 7, /**< Ethiopian Amete Alem version, aka Ethiopic Amete Alem */
        GregorianCalendar = 8, /**< Gregorian Calendar, pure proleptic implementation */
        HebrewCalendar = 9, /**< Hebrew Calendar, aka Jewish Calendar */
        //HinduCalendar = 10, /**< Hindu Lunar Calendar */
        //IslamicLunarCalendar = 11, /**< Islamic Lunar Calendar */
        IslamicCivilCalendar = 12, /**< Islamic Civil Calendar, aka Hijri, not the Lunar Calendar */
        //IslamicUmAlQuraCalendar = 13, /**< Islamic Lunar Calendar, Um Al Qura varient used in Saudi Arabia */
        IndianNationalCalendar = 14, /**< Indian National Calendar, not the Lunar Calendar */
        //Iso8601Calendar = 15, /**< ISO 8601 Standard Calendar */
        JalaliCalendar = 16, /**< Jalali Calendar, aka Persian or Iranian, also used in Afghanistan */
        //JalaliBirashkCalendar = 17, /**< Jalali Calendar, Birashk Algorythm variant */
        //Jalali33YearCalendar = 18, /**< Jalali Calendar, 33 Year cycle variant */
        JapaneseCalendar= 19, /**< Japanese Calendar, Gregorian calculation using Japanese Era (Nengô) */
        //JucheCalendar = 20, /**< Juche Calendar, used in North Korea */
        JulianCalendar = 21, /**< Julian Calendar, as used in Orthodox Churches */
        MinguoCalendar= 22, /**< Minguo Calendar, aka ROC, Republic of China or Taiwanese */
        ThaiCalendar = 23 /**< Thai Calendar, aka Buddhist or Thai Buddhist */
    };

    /**
     *
     * System used for Week Numbers
     *
     * @see setWeekNumberSystem()
     * @see weekNumberSystem()
     */
    enum WeekNumberSystem {
        DefaultWeekNumber = 1000, /**< The system locale default */
        IsoWeekNumber     =  0, /**< ISO Week Number */
        FirstFullWeek     =  1, /**< Week 1 starts on the first Week Start Day in year ends after 7 days */
        FirstPartialWeek  =  2, /**< Week 1 starts Jan 1st ends day before first Week Start Day in year */
        SimpleWeek        =  3  /**< Week 1 starts Jan 1st ends after 7 days */
    };

    /**
     *
     * The various Components that make up a Date / Time
     * In the future the Components may be combined as flags for dynamic
     * generation of Date Formats.
     *
     * @see CalendarSystem
     * @see KLocalizedDate
     * @see DateTimeComponentFormat
     */
    enum DateTimeComponent {
        Year          = 0x1,        /**< The Year portion of a date, may be number or name */
        YearName      = 0x2,        /**< The Year Name portion of a date */
        Month         = 0x4,        /**< The Month portion of a date, may be number or name */
        MonthName     = 0x8,        /**< The Month Name portion of a date */
        Day           = 0x10,       /**< The Day portion of a date, may be number or name */
        DayName       = 0x20,       /**< The Day Name portion of a date */
        JulianDay     = 0x40,       /**< The Julian Day of a date */
        EraName       = 0x80,       /**< The Era Name portion of a date */
        EraYear       = 0x100,      /**< The Era and Year portion of a date */
        YearInEra     = 0x200,      /**< The Year In Era portion of a date */
        DayOfYear     = 0x400,      /**< The Day Of Year portion of a date, may be number or name */
        DayOfYearName = 0x800,      /**< The Day Of Year Name portion of a date */
        DayOfWeek     = 0x1000,     /**< The Day Of Week / Weekday portion of a date, may be number or name */
        DayOfWeekName = 0x2000,     /**< The Day Of Week Name / Weekday Name portion of a date */
        Week          = 0x4000,     /**< The Week Number portion of a date */
        WeekYear      = 0x8000,     /**< The Week Year portion of a date */
        MonthsInYear  = 0x10000,    /**< The Months In Year portion of a date */
        WeeksInYear   = 0x20000,    /**< The Weeks In Year portion of a date */
        DaysInYear    = 0x40000,    /**< The Days In Year portion of a date */
        DaysInMonth   = 0x80000,    /**< The Days In Month portion of a date */
        DaysInWeek    = 0x100000,   /**< The Days In Week portion of a date */
        Hour          = 0x200000,   /**< The Hours portion of a date */
        Minute        = 0x400000,   /**< The Minutes portion of a date */
        Second        = 0x800000,   /**< The Seconds portion of a date */
        Millisecond   = 0x1000000,  /**< The Milliseconds portion of a date */
        DayPeriod     = 0x2000000,  /**< The Day Period portion of a date, e.g. AM/PM */
        DayPeriodHour = 0x4000000,  /**< The Day Period Hour portion of a date */
        Timezone      = 0x8000000,  /**< The Time Zone portion of a date, may be offset or name */
        TimezoneName  = 0x10000000, /**< The Time Zone Name portion of a date */
        UnixTime      = 0x20000000  /**< The UNIX Time portion of a date */
    };

    /**
     *
     * Format used for individual Date/Time Components when converted to/from a string
     * Largely equivalent to the UNICODE CLDR format width definitions 1..5
     *
     * @see DateTimeComponentFormat
     */
    enum DateTimeComponentFormat {
        DefaultComponentFormat = 1000, /**< The system locale default for the componant */
        ShortNumber = 0,             /**< Number at its natural width, e.g. 2 for the 2nd*/
        LongNumber,                  /**< Number padded to a required width, e.g. 02 for the 2nd*/
        //OrdinalNumber                /**< Ordinal number format, e.g. "2nd" for the 2nd */
        NarrowName = 3,              /**< Narrow text format, may not be unique, e.g. M for Monday */
        ShortName,                   /**< Short text format, e.g. Mon for Monday */
        LongName                     /**< Long text format, e.g. Monday for Monday */
    };

    Q_DECLARE_FLAGS(DateTimeComponents, DateTimeComponent)

    /**
     * Format for date string.
     */
    enum DateFormat {
        ShortDate,        /**< Locale Short date format, e.g. 08-04-2007 */
        LongDate,         /**< Locale Long date format, e.g. Sunday 08 April 2007 */
        FancyShortDate,   /**< Same as ShortDate for dates a week or more ago. For more
                               recent dates, it is represented as Today, Yesterday, or
                               the weekday name. */
        FancyLongDate,    /**< Same as LongDate for dates a week or more ago. For more
                               recent dates, it is represented as Today, Yesterday, or
                               the weekday name. */
        IsoDate,          /**< ISO-8601 Date format YYYY-MM-DD, e.g. 2009-12-31 */
        IsoWeekDate,      /**< ISO-8601 Week Date format YYYY-Www-D, e.g. 2009-W01-1 */
        IsoOrdinalDate    /**< ISO-8601 Ordinal Date format YYYY-DDD, e.g. 2009-001 */
    };

    /**
     * Returns a string formatted to the current locale's conventions
     * regarding dates.
     *
     * @param date the date to be formatted
     * @param format category of date format to use
     *
     * @return the date as a string
     */
    Q_INVOKABLE QString formatDate(const QDate &date, DateFormat format = LongDate) const;

    /**
     * Options for formatting date-time values.
     */
    enum DateTimeFormatOption {
        TimeZone = 0x01,    /**< Include a time zone string */
        Seconds  = 0x02     /**< Include the seconds value */
    };

    Q_DECLARE_FLAGS(DateTimeFormatOptions, DateTimeFormatOption)

    /**
     * Returns a string formatted to the current locale's conventions
     * regarding both date and time.
     *
     * @param dateTime the date and time to be formatted
     * @param format category of date format to use
     * @param options additional output options
     *
     * @return The date and time as a string
     */
    Q_INVOKABLE QString formatDateTime(const QDateTime &dateTime, DateFormat format = ShortDate,
                           DateTimeFormatOptions options = 0) const;

    /**
     * Use this to determine whether in dates a possessive form of month
     * name is preferred ("of January" rather than "January")
     *
     * @return If possessive form should be used
    */
    bool dateMonthNamePossessive() const;

    /**
     *
     * Format flags for readLocaleTime() and formatLocaleTime()
     */
    enum TimeFormatOption {
        TimeDefault        = 0x0,   ///< Default formatting using seconds and the format
                                    ///< as specified by the locale.
        TimeWithoutSeconds = 0x1,   ///< Exclude the seconds part of the time from display
        TimeWithoutAmPm    = 0x2,   ///< Read/format time string without am/pm suffix but
                                    ///< keep the 12/24h format as specified by locale time
                                    ///< format, eg. "07.33.05" instead of "07.33.05 pm" for
                                    ///< time format "%I.%M.%S %p".
        TimeDuration       = 0x6,   ///< Read/format time string as duration. This will strip
                                    ///< the am/pm suffix and read/format times with an hour
                                    ///< value of 0-23 hours, eg. "19.33.05" instead of
                                    ///< "07.33.05 pm" for time format "%I.%M.%S %p".
                                    ///< This automatically implies @c TimeWithoutAmPm.
        TimeFoldHours      = 0xE    ///< Read/format time string as duration. This will not
                                    ///< not output the hours part of the duration but will
                                    ///< add the hours (times sixty) to the number of minutes,
                                    ///< eg. "70.23" instead of "01.10.23" for time format
                                    ///< "%I.%M.%S %p".
    };

    Q_DECLARE_FLAGS(TimeFormatOptions, TimeFormatOption)

    /**
     *
     * Returns a string formatted to the current locale's conventions
     * regarding times.
     *
     * @param pTime the time to be formatted
     * @param options format option to use when formatting the time
     * @return The time as a string
     */
    Q_INVOKABLE QString formatLocaleTime(const QTime &pTime,
                             TimeFormatOptions options = Locale::TimeDefault) const;

    /**
     *
     * Returns the identifier of the digit set used to display dates and time.
     *
     * @return the digit set identifier
     * @see DigitSet
     * @see digitSetToName
     */
    DigitSet dateTimeDigitSet() const;

    /**
     * Use this to determine if the user wants a 12 hour clock.
     *
     * @return If the user wants 12h clock
     */
    bool use12Clock() const;

    /**
     *
     * Returns the Day Period matching the time given
     *
     * @param time the time to return the day period for
     * @param format the format to return teh day period in
     * @return the Day Period for the given time
     */
    Q_INVOKABLE QString dayPeriodText(const QTime &time, DateTimeComponentFormat format = DefaultComponentFormat) const;

    /**
     * Use this to determine which day is the first day of the week.
     *
     * @return an integer (Monday=1..Sunday=7)
     */
    int weekStartDay() const;

    /**
     * Use this to determine which day is the first working day of the week.
     *
     * @return an integer (Monday=1..Sunday=7)
     */
    int workingWeekStartDay() const;

    /**
     * Use this to determine which day is the last working day of the week.
     *
     * @return an integer (Monday=1..Sunday=7)
     */
    int workingWeekEndDay() const;

    /**
     * Use this to determine which day is reserved for religious observance
     *
     * @return day number (None = 0, Monday = 1, ..., Sunday = 7)
     */
    int weekDayOfPray() const;

    /**
     *
     * Returns the type of Calendar System used in this Locale
     *
     * @see Locale::CalendarSystem
     * @see CalendarSystem
     * @return the type of Calendar System
     */
    Locale::CalendarSystem calendarSystem() const;

    /**
     *
     * Sets the type of Calendar System to use in this Locale
     *
     * @see Locale::CalendarSystem
     * @see CalendarSystem
     * @param calendarSystem the Calendar System to use
     */
    void setCalendarSystem(Locale::CalendarSystem calendarSystem);

    /**
     *
     * Sets the type of Week Number System to use in this Locale
     *
     * @see Klocale::WeekNumberSystem
     * @see weekNumberSystem()
     * @param weekNumberSystem the Week Number System to use
     */
    void setWeekNumberSystem(Locale::WeekNumberSystem weekNumberSystem);

    /**
     *
     * Returns the type of Week Number System used in this Locale
     *
     * @see Klocale::WeekNumberSystem
     * @see setWeekNumberSystem()
     * @returns the Week Number System used
     */
    Locale::WeekNumberSystem weekNumberSystem() const;

    /**
     * Converts a localized monetary string to a double.
     *
     * @param numStr the string we want to convert.
     * @param ok the boolean that is set to false if it's not a number.
     *           If @p ok is 0, it will be ignored
     *
     * @return The string converted to a double
     */
    Q_INVOKABLE double readMoney(const QString &numStr) const;

    /**
     * Converts a localized numeric string to a double.
     *
     * @param numStr the string we want to convert.
     * @return The string converted to a double
     */
    Q_INVOKABLE double readNumber(const QString &numStr) const;

    /**
     * Flags for readDate()
     */
    enum ReadDateFlags {
        NormalFormat          =    1, /**< Only accept a date string in
                                           the locale LongDate format */
        ShortFormat           =    2, /**< Only accept a date string in
                                           the locale ShortDate format */
        IsoFormat             =    4, /**< Only accept a date string in
                                           ISO date format (YYYY-MM-DD) */
        IsoWeekFormat         =    8, /**< Only accept a date string in
                                           ISO Week date format (YYYY-Www-D) */
        IsoOrdinalFormat      =   16  /**< Only accept a date string in
                                           ISO Week date format (YYYY-DDD) */
    };

    /**
     * Converts a localized date string to a QDate.
     * This method is stricter than readDate(str,&ok): it will only accept
     * a date in a specific format, depending on @p flags.
     *
     * @param str the string we want to convert.
     * @param flags what format the the date string will be in
     * @return The string converted to a QDate
     * @see CalendarSystem::readDate()
     */
    Q_INVOKABLE QDate readDate(const QString &str, ReadDateFlags flags) const;

    /**
     * Converts a localized time string to a QTime.
     * This method will try to parse it with seconds, then without seconds.
     *
     * @param str the string we want to convert.
     *
     * @return The string converted to a QTime
     */
    Q_INVOKABLE QTime readTime(const QString &str) const;

    /**
     * Additional processing options for readLocaleTime().
     *
     * @remarks This is currently used as an enum but declared as a flag
     *          to be extensible
     */
    enum TimeProcessingOption {
        ProcessStrict    = 0x1,    ///< Process time in a strict manner, ie.
                                   ///< a read time string has to exactly match
                                   ///< the defined time format.
        ProcessNonStrict = 0x2     ///< Process time in a lax manner, ie.
                                   ///< allow spaces in the time-format to be
                                   ///< left out when entering a time string.
    };

    Q_DECLARE_FLAGS(TimeProcessingOptions, TimeProcessingOption)

    /**
     *
     * Converts a localized time string to a QTime.
     * This method is stricter than readTime(str, &ok) in that it will either
     * accept a time with seconds or a time without seconds.
     *
     * @param str the string we want to convert
     * @param ok the boolean that is set to false if it's not a valid time.
     *           If @p ok is 0, it will be ignored.
     * @param options format option to apply when formatting the time
     * @param processing if set to @c ProcessStrict, checking will be strict
     *               and the read time string has to have the exact time format
     *               specified. If set to @c ProcessNonStrict processing the time
     *               is lax and spaces in the time string can be left out.
     *
     * @return The string converted to a QTime
     */

    Q_INVOKABLE QTime readLocaleTime(const QString &str,
                         TimeFormatOptions options = Locale::TimeDefault,
                         TimeProcessingOptions processing = ProcessNonStrict) const;

    /**
     * Returns the language code used by this object. The domain AND the
     * library translation must be available in this language.
     * defaultLanguage() is returned by default, if no other available.
     *
     * Use languageCodeToName(language) to get human readable, localized
     * language name.
     *
     * @return the currently used language code
     *
     * @see languageCodeToName
     */
    QString language() const;

    /**
     * Returns the country code of the country where the user lives.
     *
     * The returned code complies with the ISO 3166-1 alpha-2 standard,
     * except by KDE convention it is returned in lowercase whereas the
     * official standard is uppercase.
     * See http://en.wikipedia.org/wiki/ISO_3166-1_alpha-2 for details.
     *
     * defaultCountry() is returned by default, if no other available,
     * this will always be uppercase 'C'.
     *
     * Use countryCodeToName(country) to get human readable, localized
     * country names.
     *
     * @return the country code for the user
     *
     * @see countryCodeToName
     */
    QString country() const;

    /**
     *
     * Returns the Country Division Code of the Country where the user lives.
     * When no value is set, then the Country Code will be returned.
     *
     * The returned code complies with the ISO 3166-2 standard.
     * See http://en.wikipedia.org/wiki/ISO_3166-2 for details.
     *
     * Note that unlike country() this method will return the correct case,
     * i.e. normally uppercase..
     *
     * In KDE 4.6 it is the apps responsibility to obtain a translation for the
     * code, translation and other services will be priovided in KDE 4.7.
     *
     * @return the Country Division Code for the user
     * @see setCountryDivisionCode
     */
    QString countryDivisionCode() const;

    /**
     * Returns the language codes selected by user, ordered by decreasing
     * priority.
     *
     * Use languageCodeToName(language) to get human readable, localized
     * language name.
     *
     * @return list of language codes
     *
     * @see languageCodeToName
     */
    QStringList languageList() const;

    /**
     *
     * Returns the ISO Currency Codes used in the locale, ordered by decreasing
     * priority.
     *
     * Use KCurrency::currencyCodeToName(currencyCode) to get human readable,
     * localized language name.
     *
     * @return list of ISO Currency Codes
     *
     * @see currencyCodeToName
     */
    QStringList currencyCodeList() const;

    /**
     * Returns the file encoding.
     *
     * @return The Mib of the file encoding
     *
     * @see QFile::encodeName
     * @see QFile::decodeName
     */
    int fileEncodingMib() const;

    /**
     * Changes the current date format.
     *
     * The format of the date is a string which contains variables that will
     * be replaced:
     * @li %Y with the whole year (e.g. "2004" for "2004")
     * @li %y with the lower 2 digits of the year (e.g. "04" for "2004")
     * @li %n with the month (January="1", December="12")
     * @li %m with the month with two digits (January="01", December="12")
     * @li %e with the day of the month (e.g. "1" on the first of march)
     * @li %d with the day of the month with two digits (e.g. "01" on the first of march)
     * @li %b with the short form of the month (e.g. "Jan" for January)
     * @li %B with the long form of the month (e.g. "January")
     * @li %a with the short form of the weekday (e.g. "Wed" for Wednesday)
     * @li %A with the long form of the weekday (e.g. "Wednesday" for Wednesday)
     *
     * Everything else in the format string will be taken as is.
     * For example, March 20th 1989 with the format "%y:%m:%d" results
     * in "89:03:20".
     *
     * @param format The new date format
     */
    void setDateFormat(const QString & format);

    /**
     * Changes the current short date format.
     *
     * The format of the date is a string which contains variables that will
     * be replaced:
     * @li %Y with the whole year (e.g. "1984" for "1984")
     * @li %y with the lower 2 digits of the year (e.g. "84" for "1984")
     * @li %n with the month (January="1", December="12")
     * @li %m with the month with two digits (January="01", December="12")
     * @li %e with the day of the month (e.g. "1" on the first of march)
     * @li %d with the day of the month with two digits(e.g. "01" on the first of march)
     * @li %b with the short form of the month (e.g. "Jan" for January)
     * @li %B with the long form of the month (e.g. "January")
     * @li %a with the short form of the weekday (e.g. "Wed" for Wednesday)
     * @li %A with the long form of the weekday (e.g. "Wednesday" for Wednesday)
     *
     * Everything else in the format string will be taken as is.
     * For example, March 20th 1989 with the format "%y:%m:%d" results
     * in "89:03:20".
     *
     * @param format The new short date format
     */
    void setDateFormatShort(const QString & format);

    /**
     * Changes the form of month name used in dates.
     *
     * @param possessive True if possessive forms should be used
     */
    void setDateMonthNamePossessive(bool possessive);

    /**
     * Changes the current time format.
     *
     * The format of the time is string a which contains variables that will
     * be replaced:
     * @li %H with the hour in 24h format and 2 digits (e.g. 5pm is "17", 5am is "05")
     * @li %k with the hour in 24h format and one digits (e.g. 5pm is "17", 5am is "5")
     * @li %I with the hour in 12h format and 2 digits (e.g. 5pm is "05", 5am is "05")
     * @li %l with the hour in 12h format and one digits (e.g. 5pm is "5", 5am is "5")
     * @li %M with the minute with 2 digits (e.g. the minute of 07:02:09 is "02")
     * @li %S with the seconds with 2 digits  (e.g. the minute of 07:02:09 is "09")
     * @li %p with pm or am (e.g. 17.00 is "pm", 05.00 is "am")
     *
     * Everything else in the format string will be taken as is.
     * For example, 5.23pm with the format "%H:%M" results
     * in "17:23".
     *
     * @param format The new time format
     */
    void setTimeFormat(const QString & format);

    /**
     *
     * Set digit characters used to display dates and time.
     *
     * @param digitSet the digit set identifier
     * @see DigitSet
     */
    void setDateTimeDigitSet(DigitSet digitSet);

    /**
     * Changes how KLocale defines the first day in week.
     *
     * @param day first day of the week (Monday=1..Sunday=7) as integer
     */
    void setWeekStartDay(int day);

    /**
     * Changes how KLocale defines the first working day in week.
     *
     * @param day first working day of the week (Monday=1..Sunday=7) as integer
     */
    void setWorkingWeekStartDay(int day);

    /**
     * Changes how KLocale defines the last working day in week.
     *
     * @param day last working day of the week (Monday=1..Sunday=7) as integer
     */
    void setWorkingWeekEndDay(int day);

    /**
     * Changes how KLocale defines the day reserved for religious observance.
     *
     * @param day day of the week for religious observance (None=0,Monday=1..Sunday=7) as integer
     */
    void setWeekDayOfPray(int day);

    /**
     * Returns the currently selected date format.
     *
     * @return Current date format.
     * @see setDateFormat()
     */
    QString dateFormat() const;

    /**
     * Returns the currently selected short date format.
     *
     * @return Current short date format.
     * @see setDateFormatShort()
     */
    QString dateFormatShort() const;

    /**
     * Returns the currently selected time format.
     *
     * @return Current time format.
     * @see setTimeFormat()
     */
    QString timeFormat() const;

    /**
     * Changes the symbol used to identify the decimal pointer.
     *
     * @param symbol The new decimal symbol.
     */
    void setDecimalSymbol(const QString & symbol);

    /**
     * Changes the separator used to group digits when formating numbers.
     *
     * @param separator The new thousands separator.
     */
    void setThousandsSeparator(const QString & separator);

    /**
     * Changes the sign used to identify a positive number. Normally this is
     * left blank.
     *
     * @param sign Sign used for positive numbers.
     */
    void setPositiveSign(const QString & sign);

    /**
     * Changes the sign used to identify a negative number.
     *
     * @param sign Sign used for negative numbers.
     */
    void setNegativeSign(const QString & sign);

    /**
     *
     * Changes the set of digit characters used to display numbers.
     *
     * @param digitSet the digit set identifier
     * @see DigitSet
     */
    void setDigitSet(DigitSet digitSet);

    /**
     * Changes the sign position used for positive monetary values.
     *
     * @param signpos The new sign position
     */
    void setPositiveMonetarySignPosition(SignPosition signpos);

    /**
     * Changes the sign position used for negative monetary values.
     *
     * @param signpos The new sign position
     */
    void setNegativeMonetarySignPosition(SignPosition signpos);

    /**
     * Changes the position where the currency symbol should be printed for
     * positive monetary values.
     *
     * @param prefix True if the currency symbol should be prefixed instead of
     * postfixed
     */
    void setPositivePrefixCurrencySymbol(bool prefix);

    /**
     * Changes the position where the currency symbol should be printed for
     * negative monetary values.
     *
     * @param prefix True if the currency symbol should be prefixed instead of
     * postfixed
     */
    void setNegativePrefixCurrencySymbol(bool prefix);

    /**
     *
     * Changes the number of decimal places used when formating numbers.
     *
     * @param digits The default number of digits to use.
     */
    void setDecimalPlaces(int digits);

    /**
     *
     * Changes the number of decimal places used when formating money.
     *
     * @param digits The default number of digits to use.
     */
    void setMonetaryDecimalPlaces(int digits);

    /**
     * Changes the separator used to group digits when formating monetary values.
     *
     * @param separator The new thousands separator.
     */
    void setMonetaryThousandsSeparator(const QString & separator);

    /**
     * Changes the symbol used to identify the decimal pointer for monetary
     * values.
     *
     * @param symbol The new decimal symbol.
     */
    void setMonetaryDecimalSymbol(const QString & symbol);

    /**
     *
     * Changes the current ISO Currency Code.
     *
     * @param newCurrencyCode The new Currency Code
     */
    void setCurrencyCode(const QString &newCurrencyCode);

    /**
     * Changes the current currency symbol.
     *
     * This symbol should be consistant with the selected Currency Code
     *
     * @param symbol The new currency symbol
     * @see currencyCode, KCurrency::currencySymbols
     */
    void setCurrencySymbol(const QString & symbol);

    /**
     *
     * Set digit characters used to display monetary values.
     *
     * @param digitSet the digit set identifier
     * @see DigitSet
     */
    void setMonetaryDigitSet(DigitSet digitSet);

    /**
     * Returns the preferred page size for printing.
     *
     * @return The preferred page size, cast it to QPrinter::PageSize
     */
    int pageSize() const;

    /**
     * Changes the preferred page size when printing.
     *
     * @param paperFormat the new preferred page size in the format QPrinter::PageSize
     */
    void setPageSize(int paperFormat);

    /**
     * The Metric system will give you information in mm, while the
     * Imperial system will give you information in inches.
     */
    enum MeasureSystem {
        Metric,    ///< Metric system (used e.g. in Europe)
        Imperial   ///< Imperial system (used e.g. in the United States)
    };

    /**
     * Returns which measuring system we use.
     *
     * @return The preferred measuring system
     */
    MeasureSystem measureSystem() const;

    /**
     * Changes the preferred measuring system.
     *
     * @return value The preferred measuring system
     */
    void setMeasureSystem(MeasureSystem value);

    /**
     * Translates a message as a QTranslator is supposed to.
     * The parameters are similar to i18n(), but the result
     * value has other semantics (it can be QString())
     */
    Q_INVOKABLE QString translateQt(const char *context, const char *sourceText, const char *comment) const;

    /**
     * Provides list of all known language codes.
     *
     * Use languageCodeToName(language) to get human readable, localized
     * language names.
     *
     * @return list of all language codes
     *
     * @see languageCodeToName
     * @see installedLanguages
     */
    QStringList allLanguagesList() const;

    /**
     *
     * Provides list of all installed KDE Language Translations.
     *
     * Use languageCodeToName(language) to get human readable, localized
     * language names.
     *
     * @return list of all installed language codes
     *
     * @see languageCodeToName
     */
    QStringList installedLanguages() const;

    /**
     * Convert a known language code to a human readable, localized form.
     * If an unknown language code is supplied, empty string is returned;
     * this will never happen if the code has been obtained by one of the
     * KLocale methods.
     *
     * @param language the language code
     *
     * @return the human readable and localized form if the code is known,
     *         empty otherwise
     *
     * @see language
     * @see languageList
     * @see allLanguagesList
     * @see installedLanguages
     */
    Q_INVOKABLE QString languageCodeToName(const QString &language) const;

    /**
     * Provides list of all known country codes.
     *
     * Use countryCodeToName(country) to get human readable, localized
     * country names.
     *
     * @return a list of all country codes
     *
     * @see countryCodeToName
     */
    QStringList allCountriesList() const;

    /**
     * Convert a known country code to a human readable, localized form.
     *
     * If an unknown country code is supplied, empty string is returned;
     * this will never happen if the code has been obtained by one of the
     * KLocale methods.
     *
     * @param country the country code
     *
     * @return the human readable and localized form of the country name
     *
     * @see country
     * @see allCountriesList
     */
    Q_INVOKABLE QString countryCodeToName(const QString &country) const;

    /**
     * Parses locale string into distinct parts.
     * The format of locale is language_COUNTRY@modifier.CHARSET
     *
     * @param locale the locale string to split
     * @param language set to the language part of the locale
     * @param country set to the country part of the locale
     * @param modifier set to the modifer part of the locale
     * @param charset set to the charset part of the locale
     */
    Q_INVOKABLE void splitLocale(const QString &locale, QString &language, QString &country,
                            QString &modifier, QString &charset);

    /**
     * Returns the name of the internal language.
     *
     * @return Name of the default language
     */
    QString defaultLanguage();

    /**
     * Returns the code of the default country, i.e. "C"
     *
     * This function will not provide a sensible value to use in your app,
     * please use country() instead.
     *
     * @see country
     *
     * @return Name of the default country
     */
    QString defaultCountry();

    /**
     *
     * Returns the ISO Code of the default currency.
     *
     * @return ISO Currency Code of the default currency
     */
    QString defaultCurrencyCode();

    /**
     * Reports whether evaluation of translation scripts is enabled.
     *
     * @return true if script evaluation is enabled, false otherwise.
     */
    bool useTranscript() const;

    /**
     * Checks whether or not the active catalog is found for the given language.
     *
     * @param language language to check
     */
    Q_INVOKABLE bool isApplicationTranslatedInto(const QString & language);

    /**
     *
     * Sets the Country Division Code of the Country where the user lives.
     *
     * The code must comply with the ISO 3166-2 standard.
     * See http://en.wikipedia.org/wiki/ISO_3166-2 for details.
     *
     * In KDE 4.6 it is the apps responsibility to validate the input,
     * full validation and other services will be provided in KDE 4.7.
     *
     * @param countryDivision the Country Division Code for the user
     * @return @c true on success, @c false on failure
     * @see countryDivisionCode
     */
    bool setCountryDivisionCode(const QString & countryDivision);

    /**
     *
     * Removes accelerator marker from a UI text label.
     *
     * Accelerator marker is not always a plain ampersand (&),
     * so it is not enough to just remove it by @c QString::remove().
     * The label may contain escaped markers ("&&") which must be resolved
     * and skipped, as well as CJK-style markers ("Foo (&F)") where
     * the whole parenthesis construct should be removed.
     * Therefore always use this function to remove accelerator marker
     * from UI labels.
     *
     * @param label UI label which may contain an accelerator marker
     * @return label without the accelerator marker
     */
    Q_INVOKABLE QString removeAcceleratorMarker(const QString &label) const;

    /**
     *
     * Convert all digits in the string to the given digit set.
     *
     * Conversion is normally not performed if the given digit set
     * is not appropriate in the current locale and language context.
     * Unconditional conversion may be requested by setting
     * @p ignoreContext to @c true.
     *
     * @param str the string to convert
     * @param digitSet the digit set identifier
     * @param ignoreContext unconditional conversion if @c true
     *
     * @return string with converted digits
     *
     * @see DigitSet
     */
    Q_INVOKABLE QString convertDigits(const QString &str, DigitSet digitSet,
                          bool ignoreContext = false) const;

    /**
     *
     * Reparse locale configuration files for the current selected
     * language.
     */
    Q_INVOKABLE void reparseConfiguration();

private:
    KLocale *m_locale;

Q_SIGNALS:
    void binaryUnitDialectChanged();
    void calendarSystemChanged();
    void countryDivisionCodeChanged();
    void currencyCodeChanged();
    void decimalSymbolChanged();
    void currencySymbolChanged();
    void dateFormatChanged();
    void dateFormatShortChanged();
    void dateMonthNamePossessiveChanged();
    void dateTimeDigitSetChanged();
    void decimalPlacesChanged();
    void digitSetChanged();
    void measureSystemChanged();
    void monetaryDecimalPlacesChanged();
    void monetaryDecimalSymbolChanged();
    void monetaryDigitSetChanged();
    void monetaryThousandsSeparatorChanged();
    void negativeMonetarySignPositionChanged();
    void negativePrefixCurrencySymbolChanged();
    void negativeSignChanged();
    void pageSizeChanged();
    void positiveMonetarySignPositionChanged();
    void positivePrefixCurrencySymbolChanged();
    void positiveSignChanged();
    void thousandsSeparatorChanged();
    void timeFormatChanged();
    void weekDayOfPrayChanged();
    void WeekNumberSystemChanged();
    void weekStartDayChanged();
    void workingWeekEndDayChanged();
    void workingWeekStartDayChanged();
};

#endif
