#pragma once

#include <functional>

namespace jade::tests {

/**
 * Short for Variable fix; used to quickly wrap non-pointers with RAII destructors.
 *
 * Note that if pointers are used with this class, cleanup of pointers must be performed manually in the callback.
 * However, you probably want to use std::unique_ptr instead, since you actually have a special way to delete the
 * pointer if you do this.
 */
template <typename T>
struct VFix {
    T variable;
    std::function<void(T&)> term;

    ~VFix() {
        term(variable);
    }

    T* operator->() {
        return &variable;
    }
};

}
