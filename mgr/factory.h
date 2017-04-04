#ifndef FACTORY_H
#define FACTORY_H

#include <map>
#include <string>

namespace test_functions {

/**
  * @brief Фабрика позволяющая создавать объекты наследников от общего базового класса.
  * @param base - базовый класс создаваемых объектов
  * @param derived - класс-наследник, объекты которого будем создавать
  *
  */
template <class base>
class Factory {
public:
	typedef std::shared_ptr<base> base_ptr;

	// Метод регистрирующий класс-наследник в фабрике.
	template <class derived>
	void Reg(const std::string& name) {
		obj_map[name] = base_type_ptr(new derived_type<derived>);
	}

	// Метод создающий объект
	base_ptr Build(const std::string& name) {
		return obj_map[name]->create();
	}

private:
	class base_type	{
	public:
		virtual ~base_type() {}
		virtual base_ptr create() const = 0;
	};

	typedef std::shared_ptr<base_type> base_type_ptr;

	template <class T>
	class derived_type : public base_type {
	public:

		virtual base_ptr create() const	{
			return base_ptr(new T);
		}
	};

	std::map<std::string, base_type_ptr> obj_map;  //контейнер для хранения указателей и идентификаторов

};

}

#endif // FACTORY_H
