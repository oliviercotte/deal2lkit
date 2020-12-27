//-----------------------------------------------------------
 //
 //    Copyright (C) 2014 - 2015 by the deal2lkit authors
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
 
 #ifndef _d2k_error_handler_h
 #define _d2k_error_handler_h
 
 #include <deal2lkit/config.h>
 #include <fstream>
 
 #include <deal.II/dofs/dof_handler.h>
 #include <deal.II/lac/constraint_matrix.h>
 #include <deal.II/lac/vector.h>
 
 #include <deal.II/grid/tria.h>
 
 // #include <deal.II/numerics/error_estimator.h>
 #include <deal.II/base/function.h>
 #include <deal.II/numerics/solution_transfer.h>
 #include <deal.II/numerics/data_out.h>
 
 #include <deal.II/base/convergence_table.h>
 #include <deal.II/base/logstream.h>
 #include <deal.II/base/config.h>
 
 
 #include <deal.II/base/quadrature_lib.h>
 #include <deal.II/base/utilities.h>
 
 #include <deal.II/grid/grid_tools.h>
 
 #include <deal.II/numerics/vector_tools.h>
 DEAL_II_DISABLE_EXTRA_DIAGNOSTICS
 #include <deal.II/numerics/matrix_tools.h>
 DEAL_II_ENABLE_EXTRA_DIAGNOSTICS
 #include <deal.II/numerics/data_out.h>
 #include <deal.II/fe/mapping_q.h>
 #include <deal.II/fe/fe.h>
 
 #include <deal.II/base/parameter_handler.h>
 #include <deal.II/base/conditional_ostream.h>
 
 #include <deal2lkit/parameter_acceptor.h>
 #include <map>
 #include <cstdio>
 #include <iostream>
 #include <fstream>
 #include <vector>
 #include <string>
 
 
 #include <functional>
 
 using namespace dealii;
 
 D2K_NAMESPACE_OPEN
 
 enum NormFlags
 {
   None = 0x00,
   Linfty = 0x01,
   L2 = 0x02,
   W1infty = 0x04,
   H1 = 0x08,
   AddUp = 0x10,
   Custom = 0x20
 };
 
 
 template <int ntables=1>
 class ErrorHandler : public ParameterAcceptor
 {
 public:
   ErrorHandler (const std::string name="",
                 const std::string solution_names = "u",
                 const std::string list_of_error_norms = "Linfty, L2, H1");
 
   virtual void declare_parameters(ParameterHandler &prm);
 
   virtual void parse_parameters(ParameterHandler &prm);
 
   template<typename DH, typename VEC>
   void error_from_exact(const DH &vspace,
                         const VEC &solution,
                         const Function<DH::space_dimension> &exact,
                         unsigned int table_no = 0,
                         double dt=0.);
 
 
   template<typename DH, typename VEC>
   void error_from_exact(const Mapping<DH::dimension,DH::space_dimension> &mapping,
                         const DH &vspace,
                         const VEC &solution,
                         const Function<DH::space_dimension> &exact,
                         unsigned int table_no = 0,
                         double dt=0.);
 
 
   template<typename DH>
   void custom_error(const std::function<double(const unsigned int component)> &custom_error_function,
                     const DH &dh,
                     const std::string &error_name="custom",
                     const bool add_table_extras = false,
                     const unsigned int table_no = 0,
                     const double dt=0.);
 
   template<typename DH, typename VEC>
   void difference(const DH &, const VEC &,
                   const DH &, const VEC &,
                   unsigned int table_no = 0, double dt=0.);
 
   template<typename DH, typename VEC>
   void difference(const DH &, const VEC &,
                   const VEC &, unsigned int table_no = 0, double dt=0.);
 
   void output_table(std::ostream &out=std::cout, const unsigned int table_no=0);
 
   void output_table(ConditionalOStream &pcout, const unsigned int table_no=0);
 
 private:
   const std::string solution_names;
 
   const std::string list_of_error_norms;
 
   std::vector<ConvergenceTable>  tables;
 
   std::vector<std::string> headers;
 
   std::vector<std::string> latex_headers;
 
   std::vector<std::string> latex_captions;
 
   std::vector<std::string> names;
 
   std::vector<std::vector<NormFlags> > types;
 
   bool initialized;
 
   bool compute_error;
 
   std::vector<bool> add_rates;
 
   bool write_error;
 
   bool output_error;
 
   std::string error_file_format;
 
   std::vector<std::map<std::string, bool> > extras;
 
   std::vector<std::string> rate_keys;
 
   unsigned int precision;
 };
 
 inline
 NormFlags
 operator | (NormFlags f1, NormFlags f2)
 {
   return static_cast<NormFlags> (
            static_cast<unsigned int> (f1) |
            static_cast<unsigned int> (f2));
 }
 
 inline
 NormFlags &
 operator |= (NormFlags &f1, NormFlags f2)
 {
   f1 = f1 | f2;
   return f1;
 }
 
 
 inline
 NormFlags
 operator & (NormFlags f1, NormFlags f2)
 {
   return static_cast<NormFlags> (
            static_cast<unsigned int> (f1) &
            static_cast<unsigned int> (f2));
 }
 
 
 inline
 NormFlags &
 operator &= (NormFlags &f1, NormFlags f2)
 {
   f1 = f1 & f2;
   return f1;
 }
 
 // ============================================================
 // Template instantiations
 // ============================================================
 
 
 template <int ntables>
 template<typename DH, typename VECTOR>
 void ErrorHandler<ntables>::difference(const DH &dh,
                                        const VECTOR &solution1,
                                        const VECTOR &solution2,
                                        unsigned int table_no,
                                        double dt)
 {
   AssertThrow(solution1.size() == solution2.size(), ExcDimensionMismatch(
                 solution1.size(), solution2.size()));
   VECTOR solution(solution1);
   solution -= solution2;
   error_from_exact(dh, solution,
                    ConstantFunction<DH::space_dimension>(0, headers.size()), table_no, dt);
 }
 
 
 
 template <int ntables>
 template<typename DH, typename VECTOR>
 void ErrorHandler<ntables>::error_from_exact(const DH &dh,
                                              const VECTOR &solution,
                                              const Function<DH::space_dimension> &exact,
                                              unsigned int table_no,
                                              double dt)
 {
   error_from_exact(StaticMappingQ1<DH::dimension, DH::space_dimension>::mapping,
                    dh, solution, exact, table_no, dt);
 }
 
 template <int ntables>
 template<typename DH, typename VECTOR>
 void ErrorHandler<ntables>::error_from_exact(const Mapping<DH::dimension, DH::space_dimension> &mapping,
                                              const DH &dh,
                                              const VECTOR &solution,
                                              const Function<DH::space_dimension> &exact,
                                              unsigned int table_no,
                                              double dt)
 {
   const int dim=DH::dimension;
   const int spacedim=DH::space_dimension;
   if (compute_error)
     {
       AssertThrow(initialized, ExcNotInitialized());
       AssertThrow(table_no < types.size(), ExcIndexRange(table_no, 0, names.size()));
       AssertThrow(exact.n_components == types[table_no].size(),
                   ExcDimensionMismatch(exact.n_components, types[table_no].size()));
 
       std::vector< std::vector<double> > error( exact.n_components, std::vector<double>(4));
       const unsigned int n_active_cells = dh.get_triangulation().n_global_active_cells();
       const unsigned int n_dofs=dh.n_dofs();
 
       if (extras[table_no]["cells"])
         {
           tables[table_no].add_value("cells", n_active_cells);
           tables[table_no].set_tex_caption("cells", "\\# cells");
           tables[table_no].set_tex_format("cells", "r");
         }
       if (extras[table_no]["dofs"])
         {
           tables[table_no].add_value("dofs", n_dofs);
           tables[table_no].set_tex_caption("dofs", "\\# dofs");
           tables[table_no].set_tex_format("dofs", "r");
         }
       if (extras[table_no]["dt"])
         {
           tables[table_no].add_value("dt", dt);
           tables[table_no].set_tex_caption("dt", "\\Delta t");
           tables[table_no].set_tex_format("dt", "r");
         }
 
       bool compute_Linfty = false;
       bool compute_L2 = false;
       bool compute_W1infty = false;
       bool compute_H1 = false;
       bool add_this = false;
 
       unsigned int last_non_add = 0;
 
       for (unsigned int component=0; component < exact.n_components; ++component)
         {
           NormFlags norm = types[table_no][component];
 
           // Select one Component
           ComponentSelectFunction<spacedim> select_component ( component, 1. , exact.n_components);
 
           Vector<float> difference_per_cell (dh.get_triangulation().n_global_active_cells());
 
           QGauss<dim> q_gauss((dh.get_fe().degree+1) * 2);
 
           // The add bit is set
           add_this = (norm & AddUp);
 
           if (!add_this)
             {
               last_non_add  = component;
               compute_L2  = ( norm & L2 );
               compute_H1  = ( norm & H1 );
               compute_W1infty = ( norm & W1infty ) ;
               compute_Linfty  = ( norm & Linfty );
             }
           // if add is set, we do not modify the previous selection
 
           if (compute_L2)
             {
               VectorTools::integrate_difference (
                 mapping,
                 dh,
                 solution,
                 exact,
                 difference_per_cell,
                 q_gauss,
                 VectorTools::L2_norm,
                 &select_component );
             }
 
           const double L2_error = difference_per_cell.l2_norm();
           difference_per_cell = 0;
 
           if (compute_H1)
             {
               VectorTools::integrate_difference (
                 mapping,
                 dh, //dof_handler,
                 solution,
                 exact,
                 difference_per_cell,
                 q_gauss,
                 VectorTools::H1_norm,
                 &select_component );
             }
           const double H1_error = difference_per_cell.l2_norm();
           difference_per_cell = 0;
 
           if (compute_W1infty)
             {
               VectorTools::integrate_difference (
                 mapping,
                 dh, //dof_handler,
                 solution,
                 exact,
                 difference_per_cell,
                 q_gauss,
                 VectorTools::W1infty_norm,
                 &select_component );
             }
 
           const double W1inf_error = difference_per_cell.linfty_norm();
 
           if (compute_Linfty)
             {
               VectorTools::integrate_difference (
                 mapping,
                 dh, //dof_handler,
                 solution,
                 exact,
                 difference_per_cell,
                 q_gauss,
                 VectorTools::Linfty_norm,
                 &select_component );
             }
 
           const double Linf_error = difference_per_cell.linfty_norm();
 
           if (add_this)
             {
               AssertThrow(component, ExcMessage("Cannot add on first component!"));
 
               error[last_non_add][0] = std::max(error[last_non_add][0], Linf_error);
               error[last_non_add][1] += L2_error;
               error[last_non_add][2] = std::max(error[last_non_add][2], W1inf_error);
               error[last_non_add][3] += H1_error;
 
             }
           else
             {
 
               error[component][0] = Linf_error;
               error[component][1] = L2_error;
               error[component][2] = W1inf_error;
               error[component][3] = H1_error;
 
             }
         }
 
       for (unsigned int j=0; j<exact.n_components; ++j)
         {
           NormFlags norm = types[table_no][j];
           // If this was added, don't do anything
           if ( !(norm & AddUp) )
             {
               if (norm & Linfty)
                 {
                   std::string name = headers[j] + "_Linfty";
                   std::string latex_name = "$\\| " +
                                            latex_headers[j] + " - " +
                                            latex_headers[j] +"_h \\|_\\infty $";
                   double this_error =  error[j][0];
 
                   tables[table_no].add_value(name, this_error);
                   tables[table_no].set_precision(name, precision);
                   tables[table_no].set_scientific(name, true);
                   tables[table_no].set_tex_caption(name, latex_name);
                 }
 
               if (norm & L2)
                 {
                   std::string name = headers[j] + "_L2";
                   std::string latex_name = "$\\| " +
                                            latex_headers[j] + " - " +
                                            latex_headers[j] +"_h \\|_0 $";
                   double this_error =  error[j][1];
 
                   tables[table_no].add_value(name, this_error);
                   tables[table_no].set_precision(name, precision);
                   tables[table_no].set_scientific(name, true);
                   tables[table_no].set_tex_caption(name, latex_name);
                 }
               if (norm & W1infty)
                 {
                   std::string name = headers[j] + "_W1infty";
                   std::string latex_name = "$\\| " +
                                            latex_headers[j] + " - " +
                                            latex_headers[j] +"_h \\|_{1,\\infty} $";
                   double this_error =  error[j][2];
 
                   tables[table_no].add_value(name, this_error);
                   tables[table_no].set_precision(name, precision);
                   tables[table_no].set_scientific(name, true);
                   tables[table_no].set_tex_caption(name, latex_name);
                 }
               if (norm & H1)
                 {
                   std::string name = headers[j] + "_H1";
                   std::string latex_name = "$\\| " +
                                            latex_headers[j] + " - " +
                                            latex_headers[j] +"_h \\|_1 $";
                   double this_error =  error[j][3];
 
                   tables[table_no].add_value(name, this_error);
                   tables[table_no].set_precision(name, precision);
                   tables[table_no].set_scientific(name, true);
                   tables[table_no].set_tex_caption(name, latex_name);
                 }
             }
         }
     }
 }
 
 
 
 template <int ntables>
 template<typename DH>
 void ErrorHandler<ntables>::custom_error(const std::function<double(const unsigned int component)> &custom_error_function,
                                          const DH &dh,
                                          const std::string &error_name,
                                          const bool add_table_extras,
                                          const unsigned int table_no,
                                          const double dt)
 {
   if (compute_error)
     {
       AssertThrow(initialized, ExcNotInitialized());
       AssertThrow(table_no < types.size(), ExcIndexRange(table_no, 0, types.size()));
 
       const unsigned int n_components = types.size();
       std::vector<double> c_error( types[table_no].size() );
       const unsigned int n_active_cells = dh.get_triangulation().n_global_active_cells();
       const unsigned int n_dofs=dh.n_dofs();
       if (add_table_extras)
         {
           if (extras[table_no]["cells"])
             {
               tables[table_no].add_value("cells", n_active_cells);
               tables[table_no].set_tex_caption("cells", "\\# cells");
               tables[table_no].set_tex_format("cells", "r");
             }
           if (extras[table_no]["dofs"])
             {
               tables[table_no].add_value("dofs", n_dofs);
               tables[table_no].set_tex_caption("dofs", "\\# dofs");
               tables[table_no].set_tex_format("dofs", "r");
             }
           if (extras[table_no]["dt"])
             {
               tables[table_no].add_value("dt", dt);
               tables[table_no].set_tex_caption("dt", "\\Delta t");
               tables[table_no].set_tex_format("dt", "r");
             }
         }
 
       bool add_this = false;
       bool compute_Custom = false;
 
       unsigned int last_non_add = 0;
 
       for (unsigned int component=0; component < n_components; ++component)
         {
           NormFlags norm = types[table_no][component];
 
           // The add bit is set
           add_this = (norm & AddUp);
 
           if (!add_this)
             compute_Custom  = ( norm & Custom );
           // if add is set, we do not modify the previous selection
 
           double Custom_error = 0;
 
           if (compute_Custom)
             Custom_error = custom_error_function(component);
 
           if (add_this)
             {
               AssertThrow(component, ExcMessage("Cannot add on first component!"));
 
               c_error[last_non_add] += Custom_error;
             }
           else
             {
               c_error[last_non_add] = Custom_error;
             }
         }
 
       for (unsigned int j=0; j<n_components; ++j)
         {
           NormFlags norm = types[table_no][j];
           // If this was added, don't do anything
           if ( !(norm & AddUp) )
             {
               if (norm & Custom)
                 {
                   std::string name = headers[j] + "_" +error_name;
                   std::string latex_name = "$\\| " +
                                            latex_headers[j] + " - " +
                                            latex_headers[j] +"_h \\|_{\text{"+error_name+"} $";
                   double this_error =  c_error[j];
 
                   tables[table_no].add_value(name, this_error);
                   tables[table_no].set_precision(name, precision);
                   tables[table_no].set_scientific(name, true);
                   tables[table_no].set_tex_caption(name, latex_name);
                 }
             }
         }
     }
 }
 
 
 
 D2K_NAMESPACE_CLOSE
 
 #endif
 
