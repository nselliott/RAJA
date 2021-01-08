/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   RAJA header file defining SIMD/SIMT register operations.
 *
 ******************************************************************************
 */

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-19, Lawrence Livermore National Security, LLC
// and RAJA project contributors. See the RAJA/COPYRIGHT file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#ifndef RAJA_pattern_tensor_VectorRegisterBase_HPP
#define RAJA_pattern_tensor_VectorRegisterBase_HPP

#include "RAJA/config.hpp"

#include "RAJA/util/macros.hpp"

#include "camp/camp.hpp"
#include "RAJA/pattern/tensor/internal/TensorRegisterBase.hpp"
#include "RAJA/pattern/tensor/MatrixRegister.hpp"
#include "RAJA/pattern/tensor/stats.hpp"


namespace RAJA
{


namespace internal {

  /*!
   * This provides common functionality that is special to 1D (Vector) Tensors
   */
  template<typename Derived>
  class VectorRegisterBase;

  template<typename REGISTER_POLICY, typename T, camp::idx_t SIZE, camp::idx_t ... VAL_SEQ>
  class VectorRegisterBase<TensorRegister<REGISTER_POLICY, T, VectorLayout, camp::idx_seq<SIZE>, camp::idx_seq<VAL_SEQ...>>> :
    public TensorRegisterBase<TensorRegister<REGISTER_POLICY, T, VectorLayout, camp::idx_seq<SIZE>, camp::idx_seq<VAL_SEQ...>>>
  {
    public:
      using self_type = TensorRegister<REGISTER_POLICY, T, VectorLayout, camp::idx_seq<SIZE>, camp::idx_seq<VAL_SEQ...>>;
      using base_type = TensorRegisterBase<TensorRegister<REGISTER_POLICY, T, VectorLayout, camp::idx_seq<SIZE>, camp::idx_seq<VAL_SEQ...>>>;
      using element_type = camp::decay<T>;


    private:

      RAJA_HOST_DEVICE
      RAJA_INLINE
      self_type *getThis(){
        return static_cast<self_type *>(this);
      }

      RAJA_HOST_DEVICE
      RAJA_INLINE
      constexpr
      self_type const *getThis() const{
        return static_cast<self_type const *>(this);
      }

    public:

      /*!
       * Provide left vector-matrix multiply for operator* between
       * this vector and a matrix
       */
      template<typename T2, typename L, typename RP>
      self_type
      operator*(MatrixRegister<T2, L, RP> const &y) const
      {
        return y.left_vector_multiply(*getThis());
      }

      // make sure our overloaded operator* doesn't hide our base class
      // implementation
      using base_type::operator*;


      /*!
       * @brief Performs load specified by TensorRef object.
       */
      template<typename POINTER_TYPE, typename INDEX_TYPE, internal::ET::TensorTileSize TENSOR_SIZE, camp::idx_t STRIDE_ONE_DIM>
      RAJA_INLINE
      self_type &load_ref(internal::ET::TensorRef<self_type, POINTER_TYPE, INDEX_TYPE, TENSOR_SIZE, 1, STRIDE_ONE_DIM> const &ref){

        auto ptr = ref.m_pointer + ref.m_tile.m_begin[0]*ref.m_stride[0];

        // check for packed data
        if(STRIDE_ONE_DIM == 0){
          // full vector?
          if(TENSOR_SIZE == internal::ET::TENSOR_FULL){
#ifdef RAJA_ENABLE_VECTOR_STATS
          RAJA::tensor_stats::num_vector_load_packed ++;
#endif
            getThis()->load_packed(ptr);
          }
          // partial
          else{
#ifdef RAJA_ENABLE_VECTOR_STATS
          RAJA::tensor_stats::num_vector_load_packed_n ++;
#endif
            getThis()->load_packed_n(ptr, ref.m_tile.m_size[0]);
          }

        }
        // strided data
        else
        {
          // full vector?
          if(TENSOR_SIZE == internal::ET::TENSOR_FULL){
#ifdef RAJA_ENABLE_VECTOR_STATS
          RAJA::tensor_stats::num_vector_load_strided ++;
#endif
            getThis()->load_strided(ptr, ref.m_stride[0]);
          }
          // partial
          else{
#ifdef RAJA_ENABLE_VECTOR_STATS
          RAJA::tensor_stats::num_vector_load_strided_n ++;
#endif
            getThis()->load_strided_n(ptr, ref.m_stride[0], ref.m_tile.m_size[0]);
          }
        }
        return *getThis();
      }


      /*!
       * @brief Performs load specified by TensorRef object.
       */
      template<typename POINTER_TYPE, typename INDEX_TYPE, internal::ET::TensorTileSize TENSOR_SIZE, camp::idx_t STRIDE_ONE_DIM>
      RAJA_INLINE
      self_type const &store_ref(internal::ET::TensorRef<self_type, POINTER_TYPE, INDEX_TYPE, TENSOR_SIZE, 1, STRIDE_ONE_DIM> const &ref) const {

        auto ptr = ref.m_pointer + ref.m_tile.m_begin[0]*ref.m_stride[0];

        // check for packed data
        if(STRIDE_ONE_DIM == 0){
          // full vector?
          if(TENSOR_SIZE == internal::ET::TENSOR_FULL){
#ifdef RAJA_ENABLE_VECTOR_STATS
          RAJA::tensor_stats::num_vector_store_packed ++;
#endif
            getThis()->store_packed(ptr);
          }
          // partial
          else{
#ifdef RAJA_ENABLE_VECTOR_STATS
          RAJA::tensor_stats::num_vector_store_packed_n ++;
#endif
            getThis()->store_packed_n(ptr, ref.m_tile.m_size[0]);
          }

        }
        // strided data
        else
        {
          // full vector?
          if(TENSOR_SIZE == internal::ET::TENSOR_FULL){
#ifdef RAJA_ENABLE_VECTOR_STATS
          RAJA::tensor_stats::num_vector_store_strided ++;
#endif
            getThis()->store_strided(ptr, ref.m_stride[0]);
          }
          // partial
          else{
#ifdef RAJA_ENABLE_VECTOR_STATS
          RAJA::tensor_stats::num_vector_store_strided_n ++;
#endif
            getThis()->store_strided_n(ptr, ref.m_stride[0], ref.m_tile.m_size[0]);
          }
        }
        return *getThis();
      }






      /*!
       * @brief Dot product of two vectors
       * @param x Other vector to dot with this vector
       * @return Value of (*this) dot x
       */
      RAJA_INLINE
      RAJA_HOST_DEVICE
      element_type dot(self_type const &x) const
      {
        return getThis()->multiply(x).sum();
      }



  };

} //namespace internal


}  // namespace RAJA


// Bring in the register policy file so we get the default register type
// and all of the register traits setup
#include "RAJA/policy/tensor/arch.hpp"


#endif
