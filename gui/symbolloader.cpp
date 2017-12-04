#include "symbolloader.h"

#include <algorithm>
#include <QString>
#include <QVector>
#include <QHash>
#include <QFileInfo>
#include <QFile>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

SymbolStore::SymbolStore(const QHash<QString, QVector<Symbol>>& symbols):
    m_symbols(symbols)
{
    for (auto& entry : m_symbols) {
        std::sort(entry.begin(), entry.end());
        entry.squeeze();
    }

    m_symbols.squeeze();
}

const Symbol SymbolStore::findByOffset(const QString& module, quint32 offset) const
{
    if (!m_symbols.contains(module)) {
        return Symbol();
    }

    const auto& moduleSymbols = m_symbols.value(module);
    Q_ASSERT(!moduleSymbols.empty());

    auto left = 0;
    auto right = moduleSymbols.length() - 1;

    while (left < right) {
        auto middle = (left + right) / 2;

        if (offset >= moduleSymbols[middle].offset && offset < moduleSymbols[middle + 1].offset) {
            return moduleSymbols[middle];
        }

        if (moduleSymbols[middle].offset < offset) {
            left = middle;
        } else if (moduleSymbols[middle].offset > offset) {
            right = middle;
        }
    }

    return Symbol();
}

bool SymbolLoader::load(const QString& filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't open file " << filename;
        return false;
    }

    const auto data = file.readAll();
    auto jsonDoc = QJsonDocument::fromJson(data);
    auto jsonObj = jsonDoc.object();

    QHash<QString, QVector<Symbol>> result;

    for (const auto& moduleName : jsonObj.keys()) {
        auto symArray = jsonObj.value(moduleName).toArray();
        QVector<Symbol> symbols;

        for (const auto& jsonValue : symArray) {
            auto obj = jsonValue.toObject();
            auto name = obj.value("name").toString();
            auto offset = static_cast<qint32>(obj.value("offset").toDouble());

            symbols.push_back(Symbol(offset, name));
        }

        result[moduleName] = symbols;
    }

    m_symbolStore = SymbolStore(result);

    return true;
}

const SymbolStore& SymbolLoader::getSymbols() const
{
    return m_symbolStore;
}