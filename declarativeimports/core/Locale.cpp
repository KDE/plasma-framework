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

#include "Locale.h"

/*Locale::Locale(const QString &catalog, KSharedConfig::Ptr config)
#if defined Q_WS_WIN
        : d(new LocaleWindowsPrivate(this, catalog, config))
#elif defined Q_OS_MAC
        : d(new LocaleMacPrivate(this, catalog, config))
#else
        : d(new LocaleUnixPrivate(this, catalog, config))
#endif
{
}

Locale::Locale(const QString& catalog, const QString &language, const QString &country, KConfig *config)
#if defined Q_WS_WIN
        : d(new LocaleWindowsPrivate(this, catalog, language, country, config))
#elif defined Q_OS_MAC
        : d(new LocaleMacPrivate(this, catalog, language, country, config))
#else
        : d(new LocaleUnixPrivate(this, catalog, language, country, config))
#endif
{
}*/

Locale::~Locale()
{
    delete d;
}

bool Locale::setCountry(const QString &country, KConfig *config)
{
    return d->setCountry(country, config);
    emit countryChanged();
}

bool Locale::setCountryDivisionCode(const QString &countryDivisionCode)
{
    return d->setCountryDivisionCode(countryDivisionCode);
    emit countryDivisionCodeChanged();
}

bool Locale::setLanguage(const QString &language, KConfig *config)
{
    return d->setLanguage(language, config);
    emit languageChanged();
}

bool Locale::setLanguage(const QStringList &languages)
{
    return d->setLanguage(languages);
    emit languageChanged();
}

void Locale::setCurrencyCode(const QString &newCurrencyCode)
{
    d->setCurrencyCode(newCurrencyCode);
    emit currencyCodeChanged();
}

bool Locale::isApplicationTranslatedInto(const QString &lang)
{
    return d->isApplicationTranslatedInto(lang);
}

void Locale::splitLocale(const QString &locale, QString &language, QString &country, QString &modifier,
                          QString &charset)
{
    Locale::splitLocale(locale, language, country, modifier, charset);
}

QString Locale::language() const
{
    return d->language();
}

QString Locale::country() const
{
    return d->country();
}

QString Locale::countryDivisionCode() const
{
    return d->countryDivisionCode();
}

KCurrencyCode *Locale::currency() const
{
    return d->currency();
}

QString Locale::currencyCode() const
{
    return d->currencyCode();
}

void Locale::insertCatalog(const QString &catalog)
{
    d->insertCatalog(catalog);
}

void Locale::removeCatalog(const QString &catalog)
{
    d->removeCatalog(catalog);
}

void Locale::setActiveCatalog(const QString &catalog)
{
    d->setActiveCatalog(catalog);
}

void Locale::translateRawFrom(const char *catname, const char *ctxt, const char *singular, const char *plural,
                               unsigned long n, QString *lang, QString *trans) const
{
    d->translateRawFrom(catname, ctxt, singular, plural, n, lang, trans);
}

//Convenience versions
void Locale::translateRawFrom(const char *catname, const char *msg, QString *lang, QString *trans) const
{
    d->translateRawFrom(catname, 0, msg, 0, 0, lang, trans);
}

void Locale::translateRaw(const char *msg, QString *lang, QString *trans) const
{
    d->translateRawFrom(0, 0, msg, 0, 0, lang, trans);
}

void Locale::translateRawFrom(const char *catname, const char *ctxt, const char *msg, QString *lang,
                               QString *trans) const
{
    d->translateRawFrom(catname, ctxt, msg, 0, 0, lang, trans);
}

void Locale::translateRaw(const char *ctxt, const char *msg, QString *lang, QString *trans) const
{
    d->translateRawFrom(0, ctxt, msg, 0, 0, lang, trans);
}

void Locale::translateRawFrom(const char *catname, const char *singular, const char *plural,
                               unsigned long n, QString *lang, QString *trans) const
{
    d->translateRawFrom(catname, 0, singular, plural, n, lang, trans);
}

void Locale::translateRaw(const char *singular, const char *plural, unsigned long n, QString *lang,
                           QString *trans) const
{
    d->translateRawFrom(0, 0, singular, plural, n, lang, trans);
}

void Locale::translateRaw(const char *ctxt, const char *singular, const char *plural,
                           unsigned long n, QString *lang, QString *trans) const
{
    d->translateRawFrom(0, ctxt, singular, plural, n, lang, trans);
}

QString Locale::translateQt(const char *context, const char *sourceText, const char *comment) const
{
    return d->translateQt(context, sourceText, comment);
}

QList<Locale::DigitSet> Locale::allDigitSetsList() const
{
    return d->allDigitSetsList();
}

QString Locale::digitSetToName(Locale::DigitSet digitSet, bool withDigits) const
{
    return d->digitSetToName(digitSet, withDigits);
}

QString Locale::convertDigits(const QString &str, DigitSet digitSet, bool ignoreContext) const
{
    return d->convertDigits(str, digitSet, ignoreContext);
}

bool Locale::nounDeclension() const
{
    return d->nounDeclension();
}

bool Locale::dateMonthNamePossessive() const
{
    return d->dateMonthNamePossessive();
}

int Locale::weekStartDay() const
{
    return d->weekStartDay();
}

int Locale::workingWeekStartDay() const
{
    return d->workingWeekStartDay();
}

int Locale::workingWeekEndDay() const
{
    return d->workingWeekEndDay();
}

int Locale::weekDayOfPray() const
{
    return d->weekDayOfPray();
}

int Locale::decimalPlaces() const
{
    return d->decimalPlaces();
}

QString Locale::decimalSymbol() const
{
    return d->decimalSymbol();
}

QString Locale::thousandsSeparator() const
{
    return d->thousandsSeparator();
}

QString Locale::currencySymbol() const
{
    return d->currencySymbol();
    emit currencySymbolChanged();
}

QString Locale::monetaryDecimalSymbol() const
{
    return d->monetaryDecimalSymbol();
}

QString Locale::monetaryThousandsSeparator() const
{
    return d->monetaryThousandsSeparator();
}

QString Locale::positiveSign() const
{
    return d->positiveSign();
}

QString Locale::negativeSign() const
{
    return d->negativeSign();
}

int Locale::fracDigits() const
{
    return monetaryDecimalPlaces();
}

int Locale::monetaryDecimalPlaces() const
{
    return d->monetaryDecimalPlaces();
}

bool Locale::positivePrefixCurrencySymbol() const
{
    return d->positivePrefixCurrencySymbol();
}

bool Locale::negativePrefixCurrencySymbol() const
{
    return d->negativePrefixCurrencySymbol();
}

Locale::SignPosition Locale::positiveMonetarySignPosition() const
{
    return d->positiveMonetarySignPosition();
}

Locale::SignPosition Locale::negativeMonetarySignPosition() const
{
    return d->negativeMonetarySignPosition();
}

QString Locale::formatMoney(double num, const QString &symbol, int precision) const
{
    return d->formatMoney(num, symbol, precision);
}

QString Locale::formatNumber(double num, int precision) const
{
    return d->formatNumber(num, precision);
}

QString Locale::formatLong(long num) const
{
    return d->formatLong(num);
}

QString Locale::formatNumber(const QString &numStr, bool round, int precision) const
{
    return d->formatNumber(numStr, round, precision);
}

QString Locale::formatByteSize(double size, int precision, Locale::BinaryUnitDialect dialect,
                                Locale::BinarySizeUnits specificUnit) const
{
    return d->formatByteSize(size, precision, dialect, specificUnit);
}

QString Locale::formatByteSize(double size) const
{
    return d->formatByteSize(size);
}

Locale::BinaryUnitDialect Locale::binaryUnitDialect() const
{
    return d->binaryUnitDialect();
}

void Locale::setBinaryUnitDialect(Locale::BinaryUnitDialect newDialect)
{
    d->setBinaryUnitDialect(newDialect);
    emit binaryUnitDialectChanged();
}

QString Locale::formatDuration(unsigned long mSec) const
{
    return d->formatDuration(mSec);
}

QString Locale::prettyFormatDuration(unsigned long mSec) const
{
    return d->prettyFormatDuration(mSec);
}

QString Locale::formatDate(const QDate &date, Locale::DateFormat format) const
{
    return d->formatDate(date, format);
}

void Locale::setMainCatalog(const char *catalog)
{
    Locale::setMainCatalog(catalog);
}

double Locale::readNumber(const QString &_str, bool * ok) const
{
    return d->readNumber(_str, ok);
}

double Locale::readMoney(const QString &_str, bool *ok) const
{
    return d->readMoney(_str, ok);
}

QDate Locale::readDate(const QString &intstr, bool *ok) const
{
    return d->readDate(intstr, ok);
}

QDate Locale::readDate(const QString &intstr, ReadDateFlags flags, bool *ok) const
{
    return d->readDate(intstr, flags, ok);
}

QDate Locale::readDate(const QString &intstr, const QString &fmt, bool *ok) const
{
    return d->readDate(intstr, fmt, ok);
}

QTime Locale::readTime(const QString &intstr, bool *ok) const
{
    return d->readTime(intstr, ok);
}

QTime Locale::readTime(const QString &intstr, Locale::ReadTimeFlags flags, bool *ok) const
{
    return d->readTime(intstr, flags, ok);
}

QTime Locale::readLocaleTime(const QString &intstr, bool *ok, TimeFormatOptions options,
                              TimeProcessingOptions processing) const
{
    return d->readLocaleTime(intstr, ok, options, processing);
}

QString Locale::formatTime(const QTime &time, bool includeSecs, bool isDuration) const
{
    return d->formatTime(time, includeSecs, isDuration);
}

QString Locale::formatLocaleTime(const QTime &time, TimeFormatOptions options) const
{
    return d->formatLocaleTime(time, options);
}

bool Locale::use12Clock() const
{
    return d->use12Clock();
}

QString Locale::dayPeriodText(const QTime &time, DateTimeComponentFormat format) const
{
    return d->dayPeriodForTime(time).periodName(format);
}

QStringList Locale::languageList() const
{
    return d->languageList();
}

QStringList Locale::currencyCodeList() const
{
    return d->currencyCodeList();
}

/* Just copy in for now to keep diff clean, remove later
QString Locale::formatDateTime(const Locale *locale, const QDateTime &dateTime, Locale::DateFormat format,
                                       bool includeSeconds, int daysTo, int secsTo)
{
}
*/

QString Locale::formatDateTime(const QDateTime &dateTime, Locale::DateFormat format, bool includeSeconds) const
{
    return d->formatDateTime(dateTime, format, includeSeconds);
}

QString Locale::formatDateTime(const KDateTime &dateTime, Locale::DateFormat format, DateTimeFormatOptions options) const
{
    return d->formatDateTime(dateTime, format, options);
}

QString Locale::langLookup(const QString &fname, const char *rtype)
{
    return Locale::langLookup(fname, rtype);
}
void Locale::setDateFormat(const QString &format)
{
    d->setDateFormat(format);
    emit dateFormatChanged();
}

void Locale::setDateFormatShort(const QString &format)
{
    d->setDateFormatShort(format);
    emit dateFormatShortChanged();
}

void Locale::setDateMonthNamePossessive(bool possessive)
{
    d->setDateMonthNamePossessive(possessive);
    emit dateMonthNamePossessiveChanged();
}

void Locale::setTimeFormat(const QString &format)
{
    d->setTimeFormat(format);
    emit timeFormatChanged();
}

void Locale::setWeekStartDay(int day)
{
    d->setWeekStartDay(day);
    emit weekStartDayChanged();
}

void Locale::setWorkingWeekStartDay(int day)
{
    d->setWorkingWeekStartDay(day);
    emit workingWeekStartDayChanged();
}

void Locale::setWorkingWeekEndDay(int day)
{
    d->setWorkingWeekEndDay(day);
    emit workingWeekEndDayChanged();
}

void Locale::setWeekDayOfPray(int day)
{
    d->setWeekDayOfPray(day);
    emit weekDayOfPrayChanged();
}

QString Locale::dateFormat() const
{
    return d->dateFormat();
}

QString Locale::dateFormatShort() const
{
    return d->dateFormatShort();
}

QString Locale::timeFormat() const
{
    return d->timeFormat();
}

void Locale::setDecimalPlaces(int digits)
{
    d->setDecimalPlaces(digits);
    emit decimalPlacesChanged();
}

void Locale::setDecimalSymbol(const QString &symbol)
{
    d->setDecimalSymbol(symbol);
    emit decimalSymbolChanged();
}

void Locale::setThousandsSeparator(const QString &separator)
{
    d->setThousandsSeparator(separator);
    emit thousandsSeparatorChanged();
}

void Locale::setPositiveSign(const QString &sign)
{
    d->setPositiveSign(sign);
    emit positiveSignChanged();
}

void Locale::setNegativeSign(const QString &sign)
{
    d->setNegativeSign(sign);
    emit negativeSignChanged();
}

void Locale::setPositiveMonetarySignPosition(Locale::SignPosition signpos)
{
    d->setPositiveMonetarySignPosition(signpos);
    emit positiveMonetarySignPositionChanged();
}

void Locale::setNegativeMonetarySignPosition(Locale::SignPosition signpos)
{
    d->setNegativeMonetarySignPosition(signpos);
    emit negativeMonetarySignPositionChanged();
}

void Locale::setPositivePrefixCurrencySymbol(bool prefix)
{
    d->setPositivePrefixCurrencySymbol(prefix);
    emit positivePrefixCurrencySymbolChanged();
}

void Locale::setNegativePrefixCurrencySymbol(bool prefix)
{
    d->setNegativePrefixCurrencySymbol(prefix);
    emit negativePrefixCurrencySymbolChanged();
}

void Locale::setFracDigits(int digits)
{
    setMonetaryDecimalPlaces(digits);
    emit monetaryDecimalPlacesChanged();
}

void Locale::setMonetaryDecimalPlaces(int digits)
{
    d->setMonetaryDecimalPlaces(digits);
    emit monetaryDecimalPlacesChanged();
}

void Locale::setMonetaryThousandsSeparator(const QString &separator)
{
    d->setMonetaryThousandsSeparator(separator);
    emit monetaryThousandsSeparatorChanged();
}

void Locale::setMonetaryDecimalSymbol(const QString &symbol)
{
    d->setMonetaryDecimalSymbol(symbol);
    emit monetaryDecimalSymbolChanged();
}

void Locale::setCurrencySymbol(const QString & symbol)
{
    d->setCurrencySymbol(symbol);
    emit currencySymbolChanged();
}

int Locale::pageSize() const
{
    return d->pageSize();
}

void Locale::setPageSize(int size)
{
    d->setPageSize(size);
    emit pageSizeChanged();
}

Locale::MeasureSystem Locale::measureSystem() const
{
    return d->measureSystem();
}

void Locale::setMeasureSystem(Locale::MeasureSystem value)
{
    d->setMeasureSystem(value);
    emit measureSystemChanged();
}

QString Locale::defaultLanguage()
{
    return Locale::defaultLanguage();
}

QString Locale::defaultCountry()
{
    return Locale::defaultCountry();
}

QString Locale::defaultCurrencyCode()
{
    return Locale::defaultCurrencyCode();
}

bool Locale::useTranscript() const
{
    return d->useTranscript();
}

const QByteArray Locale::encoding() const
{
    return d->encoding();
}

int Locale::encodingMib() const
{
    return d->encodingMib();
}

int Locale::fileEncodingMib() const
{
    return d->fileEncodingMib();
}

QTextCodec *Locale::codecForEncoding() const
{
    return d->codecForEncoding();
}

bool Locale::setEncoding(int mibEnum)
{
    return d->setEncoding(mibEnum);
    emit encodingChanged();
}

QStringList Locale::allLanguagesList() const
{
    return d->allLanguagesList();
}

QStringList Locale::installedLanguages() const
{
    return d->installedLanguages();
}

QString Locale::languageCodeToName(const QString &language) const
{
    return d->languageCodeToName(language);
}

QStringList Locale::allCountriesList() const
{
    return d->allCountriesList();
}

QString Locale::countryCodeToName(const QString &country) const
{
    return d->countryCodeToName(country);
}

void Locale::setCalendar(const QString &calendarType)
{
    d->setCalendar(calendarType);
    emit calendarChanged();
}

void Locale::setCalendarSystem(Locale::CalendarSystem calendarSystem)
{
    d->setCalendarSystem(calendarSystem);
    emit calendarSystemChanged();
}

QString Locale::calendarType() const
{
    return d->calendarType();
}

Locale::CalendarSystem Locale::calendarSystem() const
{
    return d->calendarSystem();
}

const KCalendarSystem * Locale::calendar() const
{
    return d->calendar();
}

void Locale::setWeekNumberSystem(Locale::WeekNumberSystem weekNumberSystem)
{
    d->setWeekNumberSystem(weekNumberSystem);
    emit WeekNumberSystemChanged();
}

Locale::WeekNumberSystem Locale::weekNumberSystem()
{
    return d->weekNumberSystem();
}

Locale::WeekNumberSystem Locale::weekNumberSystem() const
{
    return d->weekNumberSystem();
}

Locale::Locale(const Locale &rhs)
        : d(new Locale(*rhs.d))
{
    d->q = this;
}

void Locale::copyCatalogsTo(Locale *locale)
{
    d->copyCatalogsTo(locale);
}

QString Locale::localizedFilePath(const QString &filePath) const
{
    return d->localizedFilePath(filePath);
}

QString Locale::removeAcceleratorMarker(const QString &label) const
{
    return d->removeAcceleratorMarker(label);
}

void Locale::setDigitSet(Locale::DigitSet digitSet)
{
    d->setDigitSet(digitSet);
    emit digitSetChanged();
}

Locale::DigitSet Locale::digitSet() const
{
    return d->digitSet();
}

void Locale::setMonetaryDigitSet(Locale::DigitSet digitSet)
{
    d->setMonetaryDigitSet(digitSet);
    emit monetaryDigitSetChanged();
}

Locale::DigitSet Locale::monetaryDigitSet() const
{
    return d->monetaryDigitSet();
}

void Locale::setDateTimeDigitSet(Locale::DigitSet digitSet)
{
    d->setDateTimeDigitSet(digitSet);
    emit dateTimeDigitSetChanged();
}

Locale::DigitSet Locale::dateTimeDigitSet() const
{
    return d->dateTimeDigitSet();
}

void Locale::reparseConfiguration()
{
    d->initFormat();
}
