// Copyright (c) 2011-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "btcnanounits.h"

#include "primitives/transaction.h"

#include <QStringList>

BtcnanoUnits::BtcnanoUnits(QObject *parent)
    : QAbstractListModel(parent), unitlist(availableUnits()) {}

QList<BtcnanoUnits::Unit> BtcnanoUnits::availableUnits() {
    QList<BtcnanoUnits::Unit> unitlist;
    unitlist.append(BTN);
    unitlist.append(mBTN);
    unitlist.append(uBTN);
    return unitlist;
}

bool BtcnanoUnits::valid(int unit) {
    switch (unit) {
        case BTN:
        case mBTN:
        case uBTN:
            return true;
        default:
            return false;
    }
}

QString BtcnanoUnits::name(int unit) {
    switch (unit) {
        case BTN:
            return QString("BTN");
        case mBTN:
            return QString("mBTN");
        case uBTN:
            return QString::fromUtf8("μBTN");
        default:
            return QString("???");
    }
}

QString BtcnanoUnits::description(int unit) {
    switch (unit) {
        case BTN:
            return QString("Btcnanos");
        case mBTN:
            return QString("Milli-Btcnanos (1 / 1" THIN_SP_UTF8 "000)");
        case uBTN:
            return QString("Micro-Btcnanos (1 / 1" THIN_SP_UTF8
                           "000" THIN_SP_UTF8 "000)");
        default:
            return QString("???");
    }
}

qint64 BtcnanoUnits::factor(int unit) {
    switch (unit) {
        case BTN:
            return 100000000;
        case mBTN:
            return 100000;
        case uBTN:
            return 100;
        default:
            return 100000000;
    }
}

int BtcnanoUnits::decimals(int unit) {
    switch (unit) {
        case BTN:
            return 8;
        case mBTN:
            return 5;
        case uBTN:
            return 2;
        default:
            return 0;
    }
}

QString BtcnanoUnits::format(int unit, const CAmount &nIn, bool fPlus,
                             SeparatorStyle separators) {
    // Note: not using straight sprintf here because we do NOT want
    // localized number formatting.
    if (!valid(unit)) {
        // Refuse to format invalid unit
        return QString();
    }
    qint64 n = (qint64)nIn;
    qint64 coin = factor(unit);
    int num_decimals = decimals(unit);
    qint64 n_abs = (n > 0 ? n : -n);
    qint64 quotient = n_abs / coin;
    qint64 remainder = n_abs % coin;
    QString quotient_str = QString::number(quotient);
    QString remainder_str =
        QString::number(remainder).rightJustified(num_decimals, '0');

    // Use SI-style thin space separators as these are locale independent and
    // can't be confused with the decimal marker.
    QChar thin_sp(THIN_SP_CP);
    int q_size = quotient_str.size();
    if (separators == separatorAlways ||
        (separators == separatorStandard && q_size > 4)) {
        for (int i = 3; i < q_size; i += 3) {
            quotient_str.insert(q_size - i, thin_sp);
        }
    }

    if (n < 0) {
        quotient_str.insert(0, '-');
    } else if (fPlus && n > 0) {
        quotient_str.insert(0, '+');
    }

    return quotient_str + QString(".") + remainder_str;
}

// NOTE: Using formatWithUnit in an HTML context risks wrapping
// quantities at the thousands separator. More subtly, it also results
// in a standard space rather than a thin space, due to a bug in Qt's
// XML whitespace canonicalisation
//
// Please take care to use formatHtmlWithUnit instead, when
// appropriate.

QString BtcnanoUnits::formatWithUnit(int unit, const CAmount &amount,
                                     bool plussign, SeparatorStyle separators) {
    return format(unit, amount, plussign, separators) + QString(" ") +
           name(unit);
}

QString BtcnanoUnits::formatHtmlWithUnit(int unit, const CAmount &amount,
                                         bool plussign,
                                         SeparatorStyle separators) {
    QString str(formatWithUnit(unit, amount, plussign, separators));
    str.replace(QChar(THIN_SP_CP), QString(THIN_SP_HTML));
    return QString("<span style='white-space: nowrap;'>%1</span>").arg(str);
}

bool BtcnanoUnits::parse(int unit, const QString &value, CAmount *val_out) {
    if (!valid(unit) || value.isEmpty()) {
        // Refuse to parse invalid unit or empty string
        return false;
    }
    int num_decimals = decimals(unit);

    // Ignore spaces and thin spaces when parsing
    QStringList parts = removeSpaces(value).split(".");

    if (parts.size() > 2) {
        // More than one dot
        return false;
    }
    QString whole = parts[0];
    QString decimals;

    if (parts.size() > 1) {
        decimals = parts[1];
    }
    if (decimals.size() > num_decimals) {
        // Exceeds max precision
        return false;
    }
    bool ok = false;
    QString str = whole + decimals.leftJustified(num_decimals, '0');

    if (str.size() > 18) {
        // Longer numbers will exceed 63 bits
        return false;
    }
    CAmount retvalue(str.toLongLong(&ok));
    if (val_out) {
        *val_out = retvalue;
    }
    return ok;
}

QString BtcnanoUnits::getAmountColumnTitle(int unit) {
    QString amountTitle = QObject::tr("Amount");
    if (BtcnanoUnits::valid(unit)) {
        amountTitle += " (" + BtcnanoUnits::name(unit) + ")";
    }
    return amountTitle;
}

int BtcnanoUnits::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return unitlist.size();
}

QVariant BtcnanoUnits::data(const QModelIndex &index, int role) const {
    int row = index.row();
    if (row >= 0 && row < unitlist.size()) {
        Unit unit = unitlist.at(row);
        switch (role) {
            case Qt::EditRole:
            case Qt::DisplayRole:
                return QVariant(name(unit));
            case Qt::ToolTipRole:
                return QVariant(description(unit));
            case UnitRole:
                return QVariant(static_cast<int>(unit));
        }
    }
    return QVariant();
}

CAmount BtcnanoUnits::maxMoney() {
    return MAX_MONEY.GetSatoshis();
}
