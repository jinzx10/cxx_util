#ifndef __ARMA_HELPER_H__
#define __ARMA_HELPER_H__

#include <armadillo>
#include <initializer_list>
#include <regex>
#include "string_helper.h"

namespace cxut {

	template <typename T1, typename T2, char op>
	struct binary_op_return_t
	{};
	
	template <typename T1, typename T2>
	struct binary_op_return_t<T1, T2, '+'>
	{
		using return_t = decltype(std::declval<T1>() + std::declval<T2>());
	};
	
	template <typename T1, typename T2>
	struct binary_op_return_t<T1, T2, '-'>
	{
		using return_t = decltype(std::declval<T1>() - std::declval<T2>());
	};
	
	template <typename T1, typename T2>
	struct binary_op_return_t<T1, T2, '*'>
	{
		using return_t = decltype(std::declval<T1>() * std::declval<T2>());
	};
	
	template <typename T1, typename T2>
	struct binary_op_return_t<T1, T2, '/'>
	{
		using return_t = decltype(std::declval<T1>() / std::declval<T2>());
	};
	
	template <char op, typename R, typename C>
	typename std::enable_if< R::is_row && C::is_col, arma::Mat<typename binary_op_return_t<typename C::elem_type, typename R::elem_type, op>::return_t> >::type bcast_op(C const& col, R const& row) {
		switch (op) {
			case '+':
				return col + arma::repmat(row, arma::size(col).n_rows, 1).eval().each_col();
			case '-':
				return col - arma::repmat(row, arma::size(col).n_rows, 1).eval().each_col();
			case '*':
				return col * row;
			case '/':
				return col / arma::repmat(row, arma::size(col).n_rows, 1).eval().each_col();
			default:
				return {};
		}
	}
	
	template <char op, typename R, typename C>
	typename std::enable_if< R::is_row && C::is_col, arma::Mat<typename binary_op_return_t<typename R::elem_type, typename C::elem_type, op>::return_t> >::type bcast_op(R const& row, C const& col) {
		switch (op) {
			case '+':
				return arma::repmat(row, arma::size(col).n_rows, 1).eval().each_col() + col;
			case '-':
				return arma::repmat(row, arma::size(col).n_rows, 1).eval().each_col() - col;
			case '*':
				return col * row;
			case '/':
				return arma::repmat(row, arma::size(col).n_rows, 1).eval().each_col() / col;
			default:
				return {};
		}
	}
	
	
	// index range and concatenation
	inline arma::uvec range(arma::uword const& i, arma::uword const& j) {
		return arma::regspace<arma::uvec>(i, 1, j); // end-inclusive
	}
	
	template <typename T>
	arma::uvec cat(T const& i) {
		return arma::uvec{i};
	}
	
	template <typename T, typename ...Ts>
	arma::uvec cat(T const& i, Ts const& ...args) {
		return arma::join_cols(arma::uvec{i}, cat(args...));
	}
	

	// dimensions of armadillo Col/Row/Mat/Cube
	template <typename eT>
	arma::uvec dim(arma::Col<eT> const& a) {
		return arma::uvec{a.n_elem};
	}
	
	template <typename eT>
	arma::uvec dim(arma::Row<eT> const& a) {
		return arma::uvec{a.n_elem};
	}
	
	template <typename eT>
	arma::uvec dim(arma::Mat<eT> const& a) {
		return arma::uvec{a.n_rows, a.n_cols};
	}
	
	template <typename eT>
	arma::uvec dim(arma::Cube<eT> const& a) {
		return arma::uvec{a.n_rows, a.n_cols, a.n_slices};
	}
	

	// batch size setting
	template <typename eT>
	int set_size(arma::uvec const& sz, arma::Row<eT>& a) {
		return sz.n_elem == 1 ? a.set_size(sz(0)), 0 : 1;
	}
	
	template <typename eT>
	int set_size(arma::uvec const& sz, arma::Col<eT>& a) {
		return sz.n_elem == 1 ? a.set_size(sz(0)), 0 : 1;
	}
	
	template <typename eT>
	int set_size(arma::uvec const& sz, arma::Mat<eT>& a) {
		return sz.n_elem == 2 ? a.set_size(sz(0), sz(1)), 0 : 1;
	}
	
	template <typename eT>
	int set_size(arma::uvec const& sz, arma::Cube<eT>& a) {
		return sz.n_elem == 3 ? a.set_size(sz(0), sz(1), sz(2)), 0 : 1;
	}
	
	template <typename T, typename ...Ts>
	int set_size(arma::uvec const& sz, T& a, Ts& ...args) {
		int status = set_size(sz, a);
		return status ? status : set_size(sz, args...);
	}
	
	
	// matrix concatenation
	template <typename T>
	T join_r(std::initializer_list<T> m) {
		T z;
		for (auto it = m.begin(); it != m.end(); ++it)
			z = join_rows(z, *it);
		return z;
	}
	
	template <typename T>
	T join(std::initializer_list< std::initializer_list<T> > m) {
		T z;
		for (auto it = m.begin(); it != m.end(); ++it)
			z = join_cols(z, join_r(*it));
		return z;
	}
	
	template <typename T1, typename T2>
	auto join_r(T1 const& m1, T2 const& m2) {
		return arma::join_rows(m1, m2);
	}
	
	template <typename T, typename ...Ts>
	auto join_r(T const& m, Ts const& ...ms) {
	    return arma::join_rows(m, join_r(ms...)).eval(); // slow, but it needs eval() to work, why?
	}
	
	template <typename T1, typename T2>
	auto join_c(T1 const& m1, T2 const& m2) {
		return arma::join_cols(m1, m2);
	}
	
	template <typename T, typename ...Ts>
	auto join_c(T const& m, Ts const& ...ms) {
	    return arma::join_cols(m, join_c(ms...)).eval();
	}
	
	template <typename T1, typename T2>
	auto join_d(T1 const& m1, T2 const& m2) {
	    return join_cols(
				join_rows( m1, arma::zeros<arma::Mat<typename T1::elem_type>>(m1.n_rows, m2.n_cols) ),
				join_rows( arma::zeros<arma::Mat<typename T2::elem_type>>(m2.n_rows, m1.n_cols), m2 )
		).eval();
	}
	
	template <typename T, typename ...Ts>
	auto join_d(T const& m, Ts const& ...ms) {
	    return join_d(m, join_d(ms...));
	}
	

	// save/load
	template <arma::file_type F, typename T>
	void arma_save(std::string const& dir, T const& data, std::string const& name) {
	    data.save(dir+"/"+name, F); 
	}
	
	template <arma::file_type F, typename T, typename ...Ts>
	void arma_save(std::string const& dir, T const& data, std::string const& name, Ts const& ...args) {
	    data.save(dir+"/"+name, F); 
	    arma_save<F>(dir, args...);
	}
	
	template <typename T>
	void arma_load(std::string const& dir, T& data, std::string const& name) {
	    data.load(dir+"/"+name);
	}
	
	template <typename T, typename ...Ts>
	void arma_load(std::string const& dir, T& data, std::string const& name, Ts& ...args) {
	    data.load(dir+"/"+name);
	    arma_load(dir, args...);
	}


	// convert a string of brace-enclosed list of floating-point numbers to an armadillo Col
	// for complex numbers, the format must be (x,y)
	template <typename eT>
	arma::Col<eT> to_col(std::string const& str_) {
		std::string str = remove_chars(str_, " \t\n");

		if (!std::regex_match(str, std::regex(bracelist_regex(num_regex))) ) {
			std::cerr << "input string is not a valid brace-enclosed list of floating-point numbers." << std::endl;
			return {};
		}

		eT x;
		arma::Col<eT> col;
		std::stringstream ss(str.substr(1, str.size()-2));
		while (ss >> x) {
			col.insert_rows(col.n_rows, decltype(col){x});
			if (ss.peek() == ',')
				ss.ignore(1, ',');
		}
		return col;
	}

	template <typename eT>
	arma::Row<eT> to_row(std::string const& str_) {
		return to_col<eT>(str_).as_row();
	}

	template <typename eT>
	arma::Mat<eT> to_mat(std::string const& str_) {
		std::string str = remove_chars(str_, " \t\n");

		if ( !std::regex_match( str, std::regex(bracelist_regex(bracelist_regex(num_regex))) ) ) {
			std::cerr << "error: input string is not a valid brace-enclosed list of brace-enclosed list of floating-point numbers." << std::endl;
			return {};
		}

		std::regex bracelist(bracelist_regex(num_regex));
		std::smatch match;
		auto it = str.cbegin();
		arma::Mat<eT> m;
		arma::Row<eT> r;
		while (std::regex_search(it, str.cend(), match, bracelist)) {
			r = to_row<eT>(match.str());
			if (!m.is_empty() && r.n_cols != m.n_cols) {
				std::cerr << "error: inconsistent number of elements." << std::endl;	
				return {};
			}
			m.insert_rows(m.n_rows, r);
			it = match.suffix().first;
		}
		return m;
	}

	template <typename T>
	typename std::enable_if<arma::is_Row<T>::value, void>::type conv_string(std::string const& from, T& to) {
		to = to_row<typename T::elem_type>(from);
	}

	template <typename T>
	typename std::enable_if<arma::is_Col<T>::value, void>::type conv_string(std::string const& from, T& to) {
		to = to_col<typename T::elem_type>(from);
	}

	template <typename T>
	typename std::enable_if<arma::is_Mat<T>::value && !arma::is_Col<T>::value && !arma::is_Row<T>::value, void>::type conv_string(std::string const& from, T& to) {
		to = to_mat<typename T::elem_type>(from);
	}

}


#endif
