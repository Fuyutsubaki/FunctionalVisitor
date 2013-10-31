#pragma once

#include<typeinfo>
#include<functional>
#include<tuple>



template<class ...R>
struct TypeList
{
	using Types = std::tuple<R...>;
	template<size_t N>
	struct at
	{
		using type = typename  std::tuple_element<N, Types>::type;
	};
	struct size
	{
		static const size_t value = std::tuple_size<Types>::value;
	};
	struct front
	{
		using type = typename at<0>::type;
	};
	struct back
	{
		using type = typename at<size::value - 1>::type;
	};
	template<class T>
	class find
	{
		template<size_t N, bool R = std::is_same<T, at<N>::type>::value>
		struct Impl
		{
			static const size_t value = N;
		};
		template<size_t N>
		struct Impl<N, false>
		{
			static const size_t value = Impl<N - 1>::value;
		};
		template<>
		struct Impl<0, false>
		{
			static const size_t value = size::value;
		};
	public:
		static const size_t value = Impl<size::value - 1>::value;
	};
	template<class T>
	struct in
	{
		static const bool value = (find<T>::value != size::value);
	};
};


template<class ROOT,class ...LEAF>
class FunctionalVisitor
{
public:
	using LeafList = TypeList<LEAF...>;
	class Root
	{
	public:
		virtual size_t GetAccepterLeafTypeID()const = 0;
	};

	template<class Driver>
	class Leaf:public ROOT
	{
	public:
		using base_type = Leaf;
		template<class ...ArgsType>
		Leaf(ArgsType&&...args)
			:ROOT(std::forward<ArgsType>(args)...)
		{}
		size_t GetAccepterLeafTypeID()const
		{
			static_assert(LeafList::in<Driver>::value, "Faild");
			return LeafList::find<Driver>::value;
		}
	};

private:
	template<class result_type, class Functor, class ...ArgsType>
	class ImpAccept
	{
	public:
		static result_type apply_visitor(Functor&functor, const ROOT&accept, ArgsType&&...args)
		{
			typedef result_type(*Function)(Functor&functor, const ROOT&,ArgsType&&...);
			static const Function data[]
				= { static_cast<Function>(eval<LEAF>)... };
			return data[accept.GetAccepterLeafTypeID()](functor, accept, std::forward<ArgsType>(args)...);
		}
		template<class T>
		static result_type eval(Functor&functor, const ROOT&accept, ArgsType&&...args)
		{
			return functor(static_cast<const T&>(accept),args...);
		}
	};
public:
	template<class result_type, class Functor, class ...ArgsType>
	static result_type apply_visitor(Functor&functor, const ROOT&accept, ArgsType&&...args)
	{
		return ImpAccept<result_type, Functor, ArgsType...>::
			apply_visitor(functor, accept, std::forward<ArgsType>(args)...);
	}	
};

