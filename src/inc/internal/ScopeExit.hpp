//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

namespace MSIX {
    namespace details {

        template <typename TLambda>
        class lambda_call
        {
        public:
            lambda_call(const lambda_call&) = delete;
            lambda_call& operator=(const lambda_call&) = delete;
            lambda_call& operator=(lambda_call&& other) = delete;

            explicit lambda_call(TLambda&& lambda) noexcept : m_lambda(std::move(lambda))
            {
                static_assert(std::is_same<decltype(lambda()), void>::value, "scope_exit lambdas must not have a return value");
                static_assert(!std::is_lvalue_reference<TLambda>::value && !std::is_rvalue_reference<TLambda>::value,
                    "scope_exit should only be directly used with a lambda");
            }

            lambda_call(lambda_call&& other) noexcept : m_lambda(std::move(other.m_lambda)), m_call(other.m_call)
            {
                other.m_call = false;
            }

            ~lambda_call() noexcept
            {
                reset();
            }

            // Ensures the scope_exit lambda will not be called
            void release() noexcept
            {
                m_call = false;
            }

            // Executes the scope_exit lambda immediately if not yet run; ensures it will not run again
            void reset() noexcept
            {
                if (m_call)
                {
                    m_call = false;
                    m_lambda();
                }
            }

            // Returns true if the scope_exit lambda is still going to be executed
            explicit operator bool() const noexcept
            {
                return m_call;
            }

        protected:
            TLambda m_lambda;
            bool m_call = true;
        };
    } // details

    // Returns an object that executes the given lambda when destroyed.
    // Capture the object with 'auto'; use reset() to execute the lambda early or release() to avoid
    // execution.
    template <typename TLambda>
    inline auto scope_exit(TLambda&& lambda) noexcept
    {
        return details::lambda_call<TLambda>(std::forward<TLambda>(lambda));
    }
}
