// Copyright (C) Microsoft. All rights reserved.

#pragma once
#include <assert.h>

///@note All functions are pretty much passthru to BCRYPT.DLL so prototypes are identical to BCRYPT.H
///      and not necessarily compliant with AppX Coding Standards (e.g. Hungarian Notation).

namespace MsixCoreLib
{
    class BcryptLibrary
    {
    private:
        enum
        {
            OpenAlgorithmProviderFunction = 0,
            CloseAlgorithmProviderFunction,
            GetPropertyFunction,
            CreateHashFunction,
            HashDataFunction,
            FinishHashFunction,
            DestroyHashFunction,

            FunctionCount
        };

    private:
        static volatile PVOID bcryptModule;
        static FARPROC functions[FunctionCount];
        static PCSTR functionNames[FunctionCount];

    public:
        static _Check_return_ HRESULT Load();

        inline static _Must_inspect_result_ NTSTATUS BCryptOpenAlgorithmProvider(
            _Out_ BCRYPT_ALG_HANDLE* phAlgorithm,
            _In_ PCWSTR pszAlgId,
            _In_opt_ PCWSTR pszImplementation,
            _In_ ULONG dwFlags)
        {
            FARPROC function = functions[OpenAlgorithmProviderFunction];
            assert(function != NULL);
            return (((NTSTATUS(WINAPI*) (BCRYPT_ALG_HANDLE*, PCWSTR, PCWSTR, ULONG))
                (function)))(phAlgorithm, pszAlgId, pszImplementation, dwFlags);
        }

        inline static NTSTATUS BCryptCloseAlgorithmProvider(
            _Inout_ BCRYPT_ALG_HANDLE hAlgorithm,
            _In_ ULONG dwFlags)
        {
            FARPROC function = functions[CloseAlgorithmProviderFunction];
            assert(function != NULL);
            return (((NTSTATUS(WINAPI*)(BCRYPT_ALG_HANDLE, ULONG))(function)))(hAlgorithm, dwFlags);
        }

        inline static NTSTATUS BCryptGetProperty(
            _In_ BCRYPT_HANDLE hObject,
            _In_ PCWSTR pszProperty,
            _Out_writes_bytes_to_opt_(cbOutput, *pcbResult) PUCHAR pbOutput,
            _In_ ULONG cbOutput,
            _Out_ ULONG* pcbResult,
            _In_ ULONG dwFlags)
        {
            FARPROC function = functions[GetPropertyFunction];
            assert(function != NULL);
#pragma prefast(suppress:26045, "Annotations on function pointers don't work")
            return (((NTSTATUS(WINAPI*)(BCRYPT_HANDLE, PCWSTR, PUCHAR, ULONG, ULONG*, ULONG))
                (function)))(hObject, pszProperty, pbOutput, cbOutput, pcbResult, dwFlags);
        }

        inline static _Must_inspect_result_ NTSTATUS BCryptCreateHash(
            _Inout_ BCRYPT_ALG_HANDLE hAlgorithm,
            _Out_ BCRYPT_HASH_HANDLE* phHash,
            _Out_writes_bytes_all_opt_(cbHashObject) PUCHAR pbHashObject,
            _In_ ULONG cbHashObject,
            _In_reads_bytes_opt_(cbSecret) PUCHAR pbSecret,
            _In_ ULONG cbSecret,
            _In_ ULONG dwFlags)
        {
            FARPROC function = functions[CreateHashFunction];
            assert(function != NULL);
            return (((NTSTATUS(WINAPI*)(BCRYPT_ALG_HANDLE, BCRYPT_HASH_HANDLE*, PUCHAR, ULONG, PUCHAR, ULONG, ULONG))
                (function)))(hAlgorithm, phHash, pbHashObject, cbHashObject, pbSecret, cbSecret, dwFlags);
        }

        inline static _Must_inspect_result_ NTSTATUS BCryptHashData(
            _Inout_ BCRYPT_HASH_HANDLE hHash,
            _In_reads_bytes_(cbInput) PUCHAR pbInput,
            _In_ ULONG cbInput,
            _In_ ULONG dwFlags)
        {
            FARPROC function = functions[HashDataFunction];
            assert(function != NULL);
            return (((NTSTATUS(WINAPI*)(BCRYPT_HASH_HANDLE, PUCHAR, ULONG, ULONG))
                (function)))(hHash, pbInput, cbInput, dwFlags);
        }

        inline static _Must_inspect_result_ NTSTATUS BCryptFinishHash(
            _Inout_ BCRYPT_HASH_HANDLE hHash,
            _Out_writes_bytes_all_(cbOutput) PUCHAR pbOutput,
            _In_ ULONG cbOutput,
            _In_ ULONG dwFlags)
        {
            FARPROC function = functions[FinishHashFunction];
            assert(function != NULL);
            return (((NTSTATUS(WINAPI*)(BCRYPT_HASH_HANDLE, PUCHAR, ULONG, ULONG))
                (function)))(hHash, pbOutput, cbOutput, dwFlags);
        }

        inline static NTSTATUS BCryptDestroyHash(
            _Inout_ BCRYPT_HASH_HANDLE hHash)
        {
            FARPROC function = functions[DestroyHashFunction];
            assert(function != NULL);
            return (((NTSTATUS(WINAPI*)(BCRYPT_HASH_HANDLE))(function)))(hHash);
        }
    };
}
