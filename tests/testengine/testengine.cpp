/*
    SPDX-FileCopyrightText: 2008 Gilles CHAUVIN <gcnweb+kde@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "testengine.h"

#include <QBitArray>
#include <QDate>
#include <QLocale>
#include <QUrl>
#include <QBitmap>
#include <QBrush>
#include <QCursor>
#include <QFont>
#include <QIcon>
#include <QPalette>
#include <QPen>
#include <QSizePolicy>
#include <QTextFormat>

Q_DECLARE_METATYPE(TestEngine::MyUserType)

TestEngine::TestEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args)
{
} // ctor()

TestEngine::~TestEngine()
{
} // dtor()

void TestEngine::init()
{
    QString dsn(QStringLiteral("TestEngine"));

    // QVariant::Invalid
    // QVariant::BitArray
    setData(dsn, QStringLiteral("QBitArray"), QVariant(QBitArray(97, false)));
    // QVariant::Bitmap
    setData(dsn, QStringLiteral("QBitmap"), QVariant(QBitmap(12, 57)));
    // QVariant::Bool
    setData(dsn, QStringLiteral("bool"), QVariant((bool)true));
    // QVariant::Brush
    setData(dsn, QStringLiteral("QBrush"), QVariant(QBrush(Qt::SolidPattern)));
    // QVariant::ByteArray
    QByteArray byteArray;
    for (int i = 0; i < 256; ++i) {
        byteArray.append(i);
    }
    setData(dsn, QStringLiteral("QByteArray1"), QVariant(byteArray));
    setData(dsn, QStringLiteral("QByteArray2"), QVariant(QByteArray("KDE4")));
    // QVariant::Char
    setData(dsn, QStringLiteral("QChar"), QVariant(QChar(0x4B)));
    // QVariant::Color
    setData(dsn, QStringLiteral("QColor"), QVariant(QColor("#031337")));
    // QVariant::Cursor
    setData(dsn, QStringLiteral("QCursor"), QVariant(QCursor(Qt::ArrowCursor)));
    // QVariant::Date
    setData(dsn, QStringLiteral("QDate"), QVariant(QDate(2008, 1, 11)));
    // QVariant::DateTime
    setData(dsn, QStringLiteral("QDateTime"), QVariant(QDateTime(QDate(2008, 1, 11), QTime(12, 34, 56))));
    // QVariant::Double
    setData(dsn, QStringLiteral("double"), QVariant((double)12.34));
    // QVariant::Font
    setData(dsn, QStringLiteral("QFont"), QVariant(QFont()));
    // QVariant::Icon
    setData(dsn, QStringLiteral("QIcon"), QVariant(QIcon(QPixmap(12, 34))));
    // QVariant::Image
    setData(dsn, QStringLiteral("QImage"), QVariant(QImage(56, 78, QImage::Format_Mono)));
    // QVariant::Int
    setData(dsn, QStringLiteral("int"), QVariant((int) - 4321));
    // QVariant::KeySequence (???)
    // QVariant::Line
    setData(dsn, QStringLiteral("QLine"), QVariant(QLine(12, 34, 56, 78)));
    // QVariant::LineF
    setData(dsn, QStringLiteral("QLineF"), QVariant(QLineF(1.2, 3.4, 5.6, 7.8)));
    // QVariant::List
    QList<QVariant> list;
    list << QStringLiteral("KDE4") << QBrush() << QPen();
    setData(dsn, QStringLiteral("QList"), QVariant(list));
    // QVariant::Locale
    setData(dsn, QStringLiteral("QLocale"), QVariant(QLocale(QStringLiteral("fr_FR"))));
    // QVariant::LongLong
    setData(dsn, QStringLiteral("qlonglong"), QVariant((qlonglong) - 4321));
    // QVariant::Map
    QMap<QString, QVariant> map;
    for (int i = 0; i < 123; ++i) {
        QString key = QStringLiteral("key%1").arg(i);
        QString val = QStringLiteral("value%1").arg(i);
        map[key] = val;
    }
    setData(dsn, QStringLiteral("QMap"), QVariant(map));
    // QVariant::Matrix
    setData(dsn, QStringLiteral("QMatrix"), QVariant(QMatrix()));
    // QVariant::Transform
    setData(dsn, QStringLiteral("QTransform"), QVariant(QTransform()));
    // QVariant::Palette
    setData(dsn, QStringLiteral("QPalette"), QVariant(QPalette()));
    // QVariant::Pen
    setData(dsn, QStringLiteral("QPen"), QVariant(QPen(Qt::SolidLine)));
    // QVariant::Pixmap
    setData(dsn, QStringLiteral("QPixmap"), QVariant(QPixmap(12, 34)));
    // QVariant::Point
    setData(dsn, QStringLiteral("QPoint"), QVariant(QPoint(12, 34)));
    // QVariant::PointArray (obsoleted in Qt4, see QPolygon)
    // QVariant::PointF
    setData(dsn, QStringLiteral("QPointF"), QVariant(QPointF(12.34, 56.78)));
    // QVariant::Polygon
    setData(dsn, QStringLiteral("QPolygon"), QVariant(QPolygon(42)));
    // QVariant::Rect
    setData(dsn, QStringLiteral("QRect"), QVariant(QRect(12, 34, 56, 78)));
    // QVariant::RectF
    setData(dsn, QStringLiteral("QRectF"), QVariant(QRectF(1.2, 3.4, 5.6, 7.8)));
    // QVariant::RegExp
    setData(dsn, QStringLiteral("QRegExp"), QVariant(QRegExp(QStringLiteral("^KDE4$"))));
    // QVariant::Region
    setData(dsn, QStringLiteral("QRegion"), QVariant(QRegion(10, 20, 30, 40)));
    // QVariant::Size
    setData(dsn, QStringLiteral("QSize"), QVariant(QSize(12, 34)));
    // QVariant::SizeF
    setData(dsn, QStringLiteral("QSizeF"), QVariant(QSizeF(12.34, 56.78)));
    // QVariant::SizePolicy
    setData(dsn, QStringLiteral("QSizePolicy"), QVariant(QSizePolicy()));
    // QVariant::String
    setData(dsn, QStringLiteral("QString"), QVariant(QStringLiteral("KDE4 ROCKS!")));
    // QVariant::StringList
    QStringList stringList;
    stringList << QStringLiteral("K") << QStringLiteral("D") << QStringLiteral("E") << QStringLiteral("4");
    setData(dsn, QStringLiteral("QStringList"), QVariant(stringList));
    // QVariant::TextFormat
    setData(dsn, QStringLiteral("QTextFormat"), QVariant(QTextFormat()));
    // QVariant::TextLength
    setData(dsn, QStringLiteral("QTextLength"), QVariant(QTextLength()));
    // QVariant::Time
    setData(dsn, QStringLiteral("QTime"), QVariant(QTime(12, 34, 56)));
    // QVariant::UInt
    setData(dsn, QStringLiteral("uint"), QVariant((uint)4321));
    // QVariant::ULongLong
    setData(dsn, QStringLiteral("qulonglong"), QVariant((qulonglong)4321));
    // QVariant::Url
    setData(dsn, QStringLiteral("QUrl"), QVariant(QUrl(QStringLiteral("http://user:password@example.com:80/test.php?param1=foo&param2=bar"))));
    // QVariant::UserType
    MyUserType userType;
    QVariant v;
    v.setValue(userType);
    setData(dsn, QStringLiteral("UserType"), v);
} // init()

bool TestEngine::sourceRequestEvent(const QString &source)
{
    // Nothing to do...
    Q_UNUSED(source)
    return true;
} // sourceRequestEvent()

K_EXPORT_PLASMA_DATAENGINE_WITH_JSON(org.kde.examples.plasma_engine_testengine, TestEngine, "plasma-dataengine-testengine.desktop")

#include "testengine.moc"
