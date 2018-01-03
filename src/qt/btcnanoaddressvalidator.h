// Copyright (c) 2011-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BTCNANO_QT_BTCNANOADDRESSVALIDATOR_H
#define BTCNANO_QT_BTCNANOADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class BtcnanoAddressEntryValidator : public QValidator {
    Q_OBJECT

public:
    explicit BtcnanoAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

/** Btcnano address widget validator, checks for a valid btcnano address.
 */
class BtcnanoAddressCheckValidator : public QValidator {
    Q_OBJECT

public:
    explicit BtcnanoAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

#endif // BTCNANO_QT_BTCNANOADDRESSVALIDATOR_H
