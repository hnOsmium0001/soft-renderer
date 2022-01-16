#pragma once

#include "Macros.hpp"

#include <utility>

template <class TCleanupFunc>
class ScopeGuard {
private:
    TCleanupFunc mFunc;
    bool mDismissed = false;

public:
    /// Specifically left this implicit so that constructs like
    /// \code
    /// ScopeGuard sg = [&]() { res.Cleanup(); };
    /// \endcode
    /// would work. It is highly discourage and unlikely that one would want to use ScopeGuard as a function
    /// parameter, so the normal argument that implicit conversion are harmful doesn't really apply here.
    // Delibrately not explicit to allow usages like: ScopeGuard var = lambda;
    ScopeGuard(TCleanupFunc&& function) noexcept
        : mFunc{ std::move(mFunc) } {
    }

    ~ScopeGuard() noexcept {
        if (!mDismissed) {
            mFunc();
        }
    }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

    ScopeGuard(ScopeGuard&& that) noexcept
        : mFunc{ std::move(that.mFunc) } {
        that.Cancel();
    }

    ScopeGuard& operator=(ScopeGuard&& that) noexcept {
        if (!mDismissed) {
            mFunc();
        }
        this->mFunc = std::move(that.mFunc);
        this->cancelled = std::exchange(that.cancelled, true);
    }

    void Dismiss() noexcept {
        mDismissed = true;
    }
};

#define SCOPE_GUARD(name) ScopeGuard name = [&]()
#define DEFER ScopeGuard UNIQUE_NAME(scopeGuard) = [&]()
