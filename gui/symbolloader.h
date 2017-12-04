#pragma once

#include <QString>
#include <QVector>
#include <QHash>

struct Symbol
{
    qint32 offset;
    QString name;

    Symbol(qint32 offset = -1, const QString& name = "<not found>"):
        offset(offset),
        name(name)
    {
    }

    bool operator<(const Symbol& other)
    {
        return offset < other.offset;
    }
};

class SymbolStore
{
public:
    SymbolStore() = default;
    SymbolStore(const QHash<QString, QVector<Symbol>>& symbols);

    const Symbol findByOffset(const QString& module, quint32 offset) const;

private:
    QHash<QString, QVector<Symbol>> m_symbols;
};

class SymbolLoader
{
public:
    bool load(const QString& filename);

    const SymbolStore& getSymbols() const;

private:
    SymbolStore m_symbolStore;
};