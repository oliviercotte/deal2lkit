 //-----------------------------------------------------------
 //
 //    Copyright (C) 2015 by the deal2lkit authors
 //
 //    This file is part of the deal2lkit library.
 //
 //    The deal2lkit library is free software; you can use it, redistribute
 //    it, and/or modify it under the terms of the GNU Lesser General
 //    Public License as published by the Free Software Foundation; either
 //    version 2.1 of the License, or (at your option) any later version.
 //    The full text of the license can be found in the file LICENSE at
 //    the top level of the deal2lkit distribution.
 //
 //-----------------------------------------------------------
 
 #ifndef _d2k_parsed_function_h
 #define _d2k_parsed_function_h
 
 #include <deal2lkit/config.h>
 #include <deal.II/base/parsed_function.h>
 #include <deal2lkit/parameter_acceptor.h>
 
 using namespace dealii;
 
 D2K_NAMESPACE_OPEN
 
 template<int dim>
 class ParsedFunction : public ParameterAcceptor, public Functions::ParsedFunction<dim>
 {
 public:
   ParsedFunction(const std::string &name="",
                  const unsigned int &n_components=1,
                  const std::string &default_exp="",
                  const std::string &default_const="");
 
   virtual void declare_parameters(ParameterHandler &prm);
 
   virtual void parse_parameters(ParameterHandler &prm);
 
 
 private:
   const std::string default_exp;
   const std::string default_const;
   const unsigned int n_components;
 };
 
 D2K_NAMESPACE_CLOSE
 
 #endif
