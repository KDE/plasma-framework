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

#include "locale.h"
#include <KGlobal>

//TODO we need some ctors

Locale::Locale(QObject* parent)
        : QObject(parent)
{
    m_locale = new KLocale(KGlobal::locale());
}


bool Locale::setCountry(const QString &country, KConfig *config)
{
    return m_locale->setCountry(country, config);
    emit countryChanged();
}

bool Locale::setCountryDivisionCode(const QString &countryDivisionCode)
{
    return m_locale->setCountryDivisionCode(countryDivisionCode);
    emit countryDivisionCodeChanged();
}

bool Locale::setLanguage(const QString &language, KConfig *config)
{
    return m_locale->setLanguage(language, config);
    emit languageChanged();
}

bool Locale::setLanguage(const QStringList &languages)
{
    return m_locale->setLanguage(languages);
    emit languageChanged();
}

void Locale::setCurrencyCode(const QString &newCurrencyCode)
{
    m_locale->setCurrencyCode(newCurrencyCode);
    emit currencyCodeChanged();
}

bool Locale::isApplicationTranslatedInto(const QString &lang)
{
    return m_locale->isApplicationTranslatedInto(lang);
}

void Locale::splitLocale(const QString &locale, QString &language, QString &country, QString &modifier,
                          QString &charset)
{
    Locale::splitLocale(locale, language, country, modifier, charset);
}

QString Locale::language() const
{
    return m_locale->language();
}

QString Locale::country() const
{
    return m_locale->country();
}

QString Locale::countryDivisionCode() const
{
    return m_locale->countryDivisionCode();
}

KCurrencyCode *Locale::currency() const
{
    return m_locale->currency();
}

QString Locale::currencyCode() const
{
    return m_locale->currencyCode();
}

void Locale::insertCatalog(const QString &catalog)
{
    m_locale->insertCatalog(catalog);
}

void Locale::removeCatalog(const QString &catalog)
{
    m_locale->removeCatalog(catalog);
}

void Locale::setActiveCatalog(const QString &catalog)
{
    m_locale->setActiveCatalog(catalog);
}

void Locale::translateRawFrom(const char *catname, const char *ctxt, const char *singular, const char *plural,
                               unsigned long n, QString *lang, QString *trans) const
{
    m_locale->translateRawFrom(catname, ctxt, singular, plural, n, lang, trans);
}

//Convenience versions
void Locale::translateRawFrom(const char *catname, const char *msg, QString *lang, QString *trans) const
{
    m_locale->translateRawFrom(catname, 0, msg, 0, 0, lang, trans);
}

void Locale::translateRaw(const char *msg, QString *lang, QString *trans) const
{
    m_locale->translateRawFrom(0, 0, msg, 0, 0, lang, trans);
}

void Locale::translateRawFrom(const char *catname, const char *ctxt, const char *msg, QString *lang,
                               QString *trans) const
{
    m_locale->translateRawFrom(catname, ctxt, msg, 0, 0, lang, trans);
}

void Locale::translateRaw(const char *ctxt, const char *msg, QString *lang, QString *trans) const
{
    m_locale->translateRawFrom(0, ctxt, msg, 0, 0, lang, trans);
}

void Locale::translateRawFrom(const char *catname, const char *singular, const char *plural,
                               unsigned long n, QString *lang, QString *trans) const
{
    m_locale->translateRawFrom(catname, 0, singular, plural, n, lang, trans);
}

void Locale::translateRaw(const char *singular, const char *plural, unsigned long n, QString *lang,
                           QString *trans) const
{
    m_locale->translateRawFrom(0, 0, singular, plural, n, lang, trans);
}

void Locale::translateRaw(const char *ctxt, const char *singular, const char *plural,
                           unsigned long n, QString *lang, QString *trans) const
{
    m_locale->translateRawFrom(0, ctxt, singular, plural, n, lang, trans);
}

QString Locale::translateQt(const char *context, const char *sourceText, const char *comment) const
{
    return m_locale->translateQt(context, sourceText, comment);
}

QList<Locale::DigitSet> Locale::allDigitSetsList() const
{
    return m_locale->allDigitSetsList();
}

QString Locale::digitSetToName(Locale::DigitSet digitSet, bool withDigits) const
{
    return m_locale->digitSetToName(digitSet, withDigits);
}

QString Locale::convertDigits(const QString &str, DigitSet digitSet, bool ignoreContext) const
{
    return m_locale->convertDigits(str, digitSet, ignoreContext);
}

bool Locale::nounDeclension() const
{
    return m_locale->nounDeclension();
}

bool Locale::dateMonthNamePossessive() const
{
    return m_locale->dateMonthNamePossessive();
}

int Locale::weekStartDay() const
{
    return m_locale->weekStartDay();
}

int Locale::workingWeekStartDay() const
{
    return m_locale->workingWeekStartDay();
}

int Locale::workingWeekEndDay() const
{
    return m_locale->workingWeekEndDay();
}

int Locale::weekDayOfPray() const
{
    return m_locale->weekDayOfPray();
}

int Locale::decimalPlaces() const
{
    return m_locale->decimalPlaces();
}

QString Locale::decimalSymbol() const
{
    return m_locale->decimalSymbol();
}

QString Locale::thousandsSeparator() const
{
    return m_locale->thousandsSeparator();
}

QString Locale::currencySymbol() const
{
    return m_locale->currencySymbol();
    emit currencySymbolChanged();
}

QString Locale::monetaryDecimalSymbol() const
{
    return m_locale->monetaryDecimalSymbol();
}

QString Locale::monetaryThousandsSeparator() const
{
    return m_locale->monetaryThousandsSeparator();
}

QString Locale::positiveSign() const
{
    return m_locale->positiveSign();
}

QString Locale::negativeSign() const
{
    return m_locale->negativeSign();
}

int Locale::fracDigits() const
{
    return monetaryDecimalPlaces();
}

int Locale::monetaryDecimalPlaces() const
{
    return m_locale->monetaryDecimalPlaces();
}

bool Locale::positivePrefixCurrencySymbol() const
{
    return m_locale->positivePrefixCurrencySymbol();
}

bool Locale::negativePrefixCurrencySymbol() const
{
    return m_locale->negativePrefixCurrencySymbol();
}

Locale::SignPosition Locale::positiveMonetarySignPosition() const
{
    return m_locale->positiveMonetarySignPosition();
}

Locale::SignPosition Locale::negativeMonetarySignPosition() const
{
    return m_locale->negativeMonetarySignPosition();
}

QString Locale::formatMoney(double num, const QString &symbol, int precision) const
{
    return m_locale->formatMoney(num, symbol, precision);
}

QString Locale::formatNumber(double num, int precision) const
{
    return m_locale->formatNumber(num, precision);
}

QString Locale::formatLong(long num) const
{
    return m_locale->formatLong(num);
}

QString Locale::formatNumber(const QString &numStr, bool round, int precision) const
{
    return m_locale->formatNumber(numStr, round, precision);
}

QString Locale::formatByteSize(double size, int precision, Locale::BinaryUnitDialect dialect,
                                Locale::BinarySizeUnits specificUnit) const
{
    return m_locale->formatByteSize(size, precision, dialect, specificUnit);
}

QString Locale::formatByteSize(double size) const
{
    return m_locale->formatByteSize(size);
}

Locale::BinaryUnitDialect Locale::binaryUnitDialect() const
{
    return m_locale->binaryUnitDialect();
}

void Locale::setBinaryUnitDialect(Locale::BinaryUnitDialect newDialect)
{
    m_locale->setBinaryUnitDialect(newDialect);
    emit binaryUnitDialectChanged();
}

QString Locale::formatDuration(unsigned long mSec) const
{
    return m_locale->formatDuration(mSec);
}

QString Locale::prettyFormatDuration(unsigned long mSec) const
{
    return m_locale->prettyFormatDuration(mSec);
}

QString Locale::formatDate(const QDate &date, Locale::DateFormat format) const
{
    return m_locale->formatDate(date, format);
}

void Locale::setMainCatalog(const char *catalog)
{
    Locale::setMainCatalog(catalog);
}

double Locale::readNumber(const QString &_str, bool * ok) const
{
    return m_locale->readNumber(_str, ok);
}

double Locale::readMoney(const QString &_str, bool *ok) const
{
    return m_locale->readMoney(_str, ok);
}

QDate Locale::readDate(const QString &intstr, bool *ok) const
{
    return m_locale->readDate(intstr, ok);
}

QDate Locale::readDate(const QString &intstr, ReadDateFlags flags, bool *ok) const
{
    return m_locale->readDate(intstr, flags, ok);
}

QDate Locale::readDate(const QString &intstr, const QString &fmt, bool *ok) const
{
    return m_locale->readDate(intstr, fmt, ok);
}

QTime Locale::readTime(const QString &intstr, bool *ok) const
{
    return m_locale->readTime(intstr, ok);
}

QTime Locale::readTime(const QString &intstr, Locale::ReadTimeFlags flags, bool *ok) const
{
    return m_locale->readTime(intstr, flags, ok);
}

QTime Locale::readLocaleTime(const QString &intstr, bool *ok, TimeFormatOptions options,
                              TimeProcessingOptions processing) const
{
    return m_locale->readLocaleTime(intstr, ok, options, processing);
}

QString Locale::formatTime(const QTime &time, bool includeSecs, bool isDuration) const
{
    return m_locale->formatTime(time, includeSecs, isDuration);
}

QString Locale::formatLocaleTime(const QTime &time, TimeFormatOptions options) const
{
    return m_locale->formatLocaleTime(time, options);
}

bool Locale::use12Clock() const
{
    return m_locale->use12Clock();
}

QString Locale::dayPeriodText(const QTime &time, DateTimeComponentFormat format) const
{
    return m_locale->dayPeriodText(time, format);
}

QStringList Locale::languageList() const
{
    return m_locale->languageList();
}

QStringList Locale::currencyCodeList() const
{
    return m_locale->currencyCodeList();
}

/* Just copy in for now to keep diff clean, remove later
QString Locale::formatDateTime(const Locale *locale, const QDateTime &dateTime, Locale::DateFormat format,
                                       bool includeSeconds, int daysTo, int secsTo)
{
}
*/

QString Locale::formatDateTime(const QDateTime &dateTime, Locale::DateFormat format, bool includeSeconds) const
{
    return m_locale->formatDateTime(dateTime, format, includeSeconds);
}

QString Locale::formatDateTime(const KDateTime &dateTime, Locale::DateFormat format, DateTimeFormatOptions options) const
{
    return m_locale->formatDateTime(dateTime, format, options);
}

QString Locale::langLookup(const QString &fname, const char *rtype)
{
    return KLocale::langLookup(fname, rtype);
}

void Locale::setDateFormat(const QString &format)
{
    m_locale->setDateFormat(format);
    emit dateFormatChanged();
}

void Locale::setDateFormatShort(const QString &format)
{
    m_locale->setDateFormatShort(format);
    emit dateFormatShortChanged();
}

void Locale::setDateMonthNamePossessive(bool possessive)
{
    m_locale->setDateMonthNamePossessive(possessive);
    emit dateMonthNamePossessiveChanged();
}

void Locale::setTimeFormat(const QString &format)
{
    m_locale->setTimeFormat(format);
    emit timeFormatChanged();
}

void Locale::setWeekStartDay(int day)
{
    m_locale->setWeekStartDay(day);
    emit weekStartDayChanged();
}

void Locale::setWorkingWeekStartDay(int day)
{
    m_locale->setWorkingWeekStartDay(day);
    emit workingWeekStartDayChanged();
}

void Locale::setWorkingWeekEndDay(int day)
{
    m_locale->setWorkingWeekEndDay(day);
    emit workingWeekEndDayChanged();
}

void Locale::setWeekDayOfPray(int day)
{
    m_locale->setWeekDayOfPray(day);
    emit weekDayOfPrayChanged();
}

QString Locale::dateFormat() const
{
    return m_locale->dateFormat();
}

QString Locale::dateFormatShort() const
{
    return m_locale->dateFormatShort();
}

QString Locale::timeFormat() const
{
    return m_locale->timeFormat();
}

void Locale::setDecimalPlaces(int digits)
{
    m_locale->setDecimalPlaces(digits);
    emit decimalPlacesChanged();
}

void Locale::setDecimalSymbol(const QString &symbol)
{
    m_locale->setDecimalSymbol(symbol);
    emit decimalSymbolChanged();
}

void Locale::setThousandsSeparator(const QString &separator)
{
    m_locale->setThousandsSeparator(separator);
    emit thousandsSeparatorChanged();
}

void Locale::setPositiveSign(const QString &sign)
{
    m_locale->setPositiveSign(sign);
    emit positiveSignChanged();
}

void Locale::setNegativeSign(const QString &sign)
{
    m_locale->setNegativeSign(sign);
    emit negativeSignChanged();
}

void Locale::setPositiveMonetarySignPosition(Locale::SignPosition signpos)
{
    m_locale->setPositiveMonetarySignPosition(signpos);
    emit positiveMonetarySignPositionChanged();
}

void Locale::setNegativeMonetarySignPosition(Locale::SignPosition signpos)
{
    m_locale->setNegativeMonetarySignPosition(signpos);
    emit negativeMonetarySignPositionChanged();
}

void Locale::setPositivePrefixCurrencySymbol(bool prefix)
{
    m_locale->setPositivePrefixCurrencySymbol(prefix);
    emit positivePrefixCurrencySymbolChanged();
}

void Locale::setNegativePrefixCurrencySymbol(bool prefix)
{
    m_locale->setNegativePrefixCurrencySymbol(prefix);
    emit negativePrefixCurrencySymbolChanged();
}

void Locale::setFracDigits(int digits)
{
    setMonetaryDecimalPlaces(digits);
    emit monetaryDecimalPlacesChanged();
}

void Locale::setMonetaryDecimalPlaces(int digits)
{
    m_locale->setMonetaryDecimalPlaces(digits);
    emit monetaryDecimalPlacesChanged();
}

void Locale::setMonetaryThousandsSeparator(const QString &separator)
{
    m_locale->setMonetaryThousandsSeparator(separator);
    emit monetaryThousandsSeparatorChanged();
}

void Locale::setMonetaryDecimalSymbol(const QString &symbol)
{
    m_locale->setMonetaryDecimalSymbol(symbol);
    emit monetaryDecimalSymbolChanged();
}

void Locale::setCurrencySymbol(const QString & symbol)
{
    m_locale->setCurrencySymbol(symbol);
    emit currencySymbolChanged();
}

int Locale::pageSize() const
{
    return m_locale->pageSize();
}

void Locale::setPageSize(int size)
{
    m_locale->setPageSize(size);
    emit pageSizeChanged();
}

Locale::MeasureSystem Locale::measureSystem() const
{
    return m_locale->measureSystem();
}

void Locale::setMeasureSystem(Locale::MeasureSystem value)
{
    m_locale->setMeasureSystem(value);
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
    return m_locale->useTranscript();
}

const QByteArray Locale::encoding() const
{
    return m_locale->encoding();
}

int Locale::encodingMib() const
{
    return m_locale->encodingMib();
}

int Locale::fileEncodingMib() const
{
    return m_locale->fileEncodingMib();
}

QTextCodec *Locale::codecForEncoding() const
{
    return m_locale->codecForEncoding();
}

bool Locale::setEncoding(int mibEnum)
{
    return m_locale->setEncoding(mibEnum);
    emit encodingChanged();
}

QStringList Locale::allLanguagesList() const
{
    return m_locale->allLanguagesList();
}

QStringList Locale::installedLanguages() const
{
    return m_locale->installedLanguages();
}

QString Locale::languageCodeToName(const QString &language) const
{
    return m_locale->languageCodeToName(language);
}

QStringList Locale::allCountriesList() const
{
    return m_locale->allCountriesList();
}

QString Locale::countryCodeToName(const QString &country) const
{
    return m_locale->countryCodeToName(country);
}

void Locale::setCalendar(const QString &calendarType)
{
    m_locale->setCalendar(calendarType);
    emit calendarChanged();
}

void Locale::setCalendarSystem(Locale::CalendarSystem calendarSystem)
{
    m_locale->setCalendarSystem(calendarSystem);
    emit calendarSystemChanged();
}

QString Locale::calendarType() const
{
    return m_locale->calendarType();
}

Locale::CalendarSystem Locale::calendarSystem() const
{
    return m_locale->calendarSystem();
}

const KCalendarSystem * Locale::calendar() const
{
    return m_locale->calendar();
}

void Locale::setWeekNumberSystem(Locale::WeekNumberSystem weekNumberSystem)
{
    m_locale->setWeekNumberSystem(weekNumberSystem);
    emit WeekNumberSystemChanged();
}

Locale::WeekNumberSystem Locale::weekNumberSystem()
{
    return m_locale->weekNumberSystem();
}

Locale::WeekNumberSystem Locale::weekNumberSystem() const
{
    return m_locale->weekNumberSystem();
}

/*Locale::Locale(const Locale &rhs)
        : d(new Locale(*rhs.d))
{
    m_locale->q = this;
}*/

void Locale::copyCatalogsTo(Locale *locale)
{
    m_locale->copyCatalogsTo(locale);
}

QString Locale::localizedFilePath(const QString &filePath) const
{
    return m_locale->localizedFilePath(filePath);
}

QString Locale::removeAcceleratorMarker(const QString &label) const
{
    return m_locale->removeAcceleratorMarker(label);
}

void Locale::setDigitSet(Locale::DigitSet digitSet)
{
    m_locale->setDigitSet(digitSet);
    emit digitSetChanged();
}

Locale::DigitSet Locale::digitSet() const
{
    return m_locale->digitSet();
}

void Locale::setMonetaryDigitSet(Locale::DigitSet digitSet)
{
    m_locale->setMonetaryDigitSet(digitSet);
    emit monetaryDigitSetChanged();
}

Locale::DigitSet Locale::monetaryDigitSet() const
{
    return m_locale->monetaryDigitSet();
}

void Locale::setDateTimeDigitSet(Locale::DigitSet digitSet)
{
    m_locale->setDateTimeDigitSet(digitSet);
    emit dateTimeDigitSetChanged();
}

Locale::DigitSet Locale::dateTimeDigitSet() const
{
    return m_locale->dateTimeDigitSet();
}

void Locale::reparseConfiguration()
{
    m_locale->reparseConfiguration();
}
