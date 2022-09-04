#pragma once
#include <functional>
#include <map>
#include <string>

#include "Operator.h"

#define REGISTER_OPERATOR(op_type, create_func) \
    bool op_type##_registered = OperatorRegistry::registerOperator(#op_type, (create_func))

namespace Op
{
    class OperatorRegistry
    {
    public:
        typedef std::function<Operator *()> FactoryFunction;
        typedef std::map<std::string, FactoryFunction> FactoryMap;

        static bool registerOperator(const std::string &type, FactoryFunction func)
        {
            FactoryMap *map = getFactoryMap();
            if (map->find(type) != map->end())
            {
                return false;
            }
            (*map)[type] = func;
            return true;
        }

        static Operator *create(const std::string &type)
        {
            FactoryMap *map = getFactoryMap();
            if (map->find(type) == map->end())
            {
                return nullptr;
            }
            Operator *op = (*map)[type]();
            op->setType(type);
            return op;
        }

        class key_iterator : public FactoryMap::const_iterator
        {
        public:
            key_iterator() : FactoryMap::const_iterator() {}
            key_iterator(FactoryMap::const_iterator it) : FactoryMap::const_iterator(it) {}
            const std::string *operator->() { return &(FactoryMap::const_iterator::operator->()->first); }
            const std::string &operator*() { return FactoryMap::const_iterator::operator*().first; }
        };

        static key_iterator cbegin()
        {
            FactoryMap *map = getFactoryMap();
            return map->cbegin();
        }

        static key_iterator cend()
        {
            FactoryMap *map = getFactoryMap();
            return map->cend();
        }

    private:
        static FactoryMap *getFactoryMap()
        {
            static FactoryMap map;
            return &map;
        }
    };
}
