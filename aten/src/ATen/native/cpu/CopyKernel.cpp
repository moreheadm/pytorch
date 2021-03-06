#include <ATen/ATen.h>

#include <ATen/Dispatch.h>
#include <ATen/native/Copy.h>
#include <ATen/native/TensorIterator.h>
#include <ATen/native/cpu/Loops.h>

namespace at {
namespace native {
namespace {

template <typename self_T>
void copy_kernel_cast(TensorIterator& iter) {
  AT_DISPATCH_ALL_TYPES_AND2(
      ScalarType::Half,
      ScalarType::Bool,
      iter.dtype(1),
      "copy_kernel_cast",
      [&] {
        at::native::unary_kernel(iter, [=](scalar_t a) -> self_T {
          return static_cast<self_T>(
              static_cast<at::native::inter_copy_type_t<self_T>>(a));
        });
      });
}

static void copy_kernel(TensorIterator& iter, bool non_blocking) {
  ScalarType dtype = iter.dtype(0);
  if (dtype == iter.dtype(1)) {
    if (dtype == ScalarType::Half) {
      unary_kernel(iter, [=](at::Half a) -> at::Half { return a; });
    } else {
      AT_DISPATCH_ALL_TYPES_AND(
          ScalarType::Bool, dtype, "copy_kernel", [&] {
            unary_kernel_vec(
                iter,
                [=](scalar_t a) -> scalar_t { return a; },
                [=](Vec256<scalar_t> a) { return a; });
          });
    }
  } else {
    AT_DISPATCH_ALL_TYPES_AND2(ScalarType::Half, ScalarType::Bool, dtype, "copy_", [&] {
      copy_kernel_cast<scalar_t>(iter);
    });
  }
}

} // anonymous namespace

REGISTER_DISPATCH(copy_stub, &copy_kernel);

} // namespace native
} // namespace at
