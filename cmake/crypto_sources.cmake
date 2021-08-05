# kudos to https://github.com/madwax/3ndparty.cmake.openssl/ for the starting point!
set(XRT "${TARGET_SOURCE_DIR_TRUE}/" ) # do we even use this?
set(CRYPTO "${OpenSSL_SOURCE_PATH}/crypto")
set(CRYPTO_INC "${OpenSSL_SOURCE_PATH}/include/crypto")
set(OPENSSL_INC "${OpenSSL_SOURCE_PATH}/include/openssl")
set(INTERNAL_INC "${OpenSSL_SOURCE_PATH}/include/internal")
# crypto
list(APPEND XSRC
    ${CRYPTO}/cpt_err.c
    ${CRYPTO}/cryptlib.c
    ${CRYPTO}/ctype.c
    ${CRYPTO}/cversion.c
    ${CRYPTO}/ex_data.c
    ${CRYPTO}/getenv.c
    ${CRYPTO}/init.c
    ${CRYPTO}/mem.c
    ${CRYPTO}/mem_clr.c
    ${CRYPTO}/mem_dbg.c
    ${CRYPTO}/mem_sec.c
    ${CRYPTO}/o_dir.c
    ${CRYPTO}/o_fopen.c
    ${CRYPTO}/o_init.c
    ${CRYPTO}/o_str.c
    ${CRYPTO}/o_time.c
    ${CRYPTO}/uid.c

    ${CRYPTO}/aes/aes_cbc.c
    ${CRYPTO}/aes/aes_cfb.c
    ${CRYPTO}/aes/aes_core.c
    ${CRYPTO}/aes/aes_ecb.c
    ${CRYPTO}/aes/aes_ige.c
    ${CRYPTO}/aes/aes_misc.c
    ${CRYPTO}/aes/aes_ofb.c
    ${CRYPTO}/aes/aes_wrap.c

    ${CRYPTO}/aria/aria.c

    ${CRYPTO}/asn1/a_bitstr.c
    ${CRYPTO}/asn1/a_d2i_fp.c
    ${CRYPTO}/asn1/a_digest.c
    ${CRYPTO}/asn1/a_dup.c
    ${CRYPTO}/asn1/a_gentm.c
    ${CRYPTO}/asn1/a_i2d_fp.c
    ${CRYPTO}/asn1/a_int.c
    ${CRYPTO}/asn1/a_mbstr.c
    ${CRYPTO}/asn1/a_object.c
    ${CRYPTO}/asn1/a_octet.c
    ${CRYPTO}/asn1/a_print.c
    ${CRYPTO}/asn1/a_sign.c
    ${CRYPTO}/asn1/a_strex.c
    ${CRYPTO}/asn1/a_strnid.c
    ${CRYPTO}/asn1/a_time.c
    ${CRYPTO}/asn1/a_type.c
    ${CRYPTO}/asn1/a_utctm.c
    ${CRYPTO}/asn1/a_utf8.c
    ${CRYPTO}/asn1/a_verify.c
    ${CRYPTO}/asn1/ameth_lib.c
    ${CRYPTO}/asn1/asn_mstbl.c
    ${CRYPTO}/asn1/asn1_err.c
    ${CRYPTO}/asn1/asn1_gen.c
    ${CRYPTO}/asn1/asn1_item_list.c
    ${CRYPTO}/asn1/asn1_lib.c
    ${CRYPTO}/asn1/asn1_par.c
    ${CRYPTO}/asn1/asn_mime.c
    ${CRYPTO}/asn1/asn_moid.c
    ${CRYPTO}/asn1/asn_pack.c
    ${CRYPTO}/asn1/bio_asn1.c
    ${CRYPTO}/asn1/bio_ndef.c
    ${CRYPTO}/asn1/d2i_pr.c
    ${CRYPTO}/asn1/d2i_pu.c
    ${CRYPTO}/asn1/evp_asn1.c
    ${CRYPTO}/asn1/f_int.c
    ${CRYPTO}/asn1/f_string.c
    ${CRYPTO}/asn1/i2d_pr.c
    ${CRYPTO}/asn1/i2d_pu.c
#    ${CRYPTO}/asn1/n_pkey.c OPENSSL_NO_RC4
    ${CRYPTO}/asn1/nsseq.c
    ${CRYPTO}/asn1/p5_pbe.c
    ${CRYPTO}/asn1/p5_pbev2.c
    ${CRYPTO}/asn1/p8_pkey.c
    ${CRYPTO}/asn1/t_bitst.c
    ${CRYPTO}/asn1/t_pkey.c
    ${CRYPTO}/asn1/t_spki.c
    ${CRYPTO}/asn1/tasn_scn.c
    ${CRYPTO}/asn1/tasn_dec.c
    ${CRYPTO}/asn1/tasn_enc.c
    ${CRYPTO}/asn1/tasn_fre.c
    ${CRYPTO}/asn1/tasn_new.c
    ${CRYPTO}/asn1/tasn_prn.c
    ${CRYPTO}/asn1/tasn_typ.c
    ${CRYPTO}/asn1/tasn_utl.c
    ${CRYPTO}/asn1/x_algor.c
    ${CRYPTO}/asn1/x_bignum.c
    ${CRYPTO}/asn1/x_info.c
    ${CRYPTO}/asn1/x_int64.c
    ${CRYPTO}/asn1/x_long.c
    ${CRYPTO}/asn1/x_pkey.c
    ${CRYPTO}/asn1/x_sig.c
    ${CRYPTO}/asn1/x_spki.c
    ${CRYPTO}/asn1/x_val.c

    ${CRYPTO}/async/async.c
    ${CRYPTO}/async/async_err.c
    ${CRYPTO}/async/async_wait.c
    ${CRYPTO}/async/arch/async_null.c  

    ${CRYPTO}/bio/b_dump.c
    ${CRYPTO}/bio/b_print.c
    ${CRYPTO}/bio/bf_buff.c
    ${CRYPTO}/bio/bf_nbio.c
    ${CRYPTO}/bio/bf_null.c
    ${CRYPTO}/bio/bio_cb.c
    ${CRYPTO}/bio/bio_err.c
    ${CRYPTO}/bio/bio_lib.c
    ${CRYPTO}/bio/bio_meth.c
#    ${CRYPTO}/bio/bss_acpt.c OPENSSL_NO_SOCK
    ${CRYPTO}/bio/bss_bio.c
#    ${CRYPTO}/bio/bss_conn.c OPENSSL_NO_SOCK
    ${CRYPTO}/bio/bss_fd.c
    ${CRYPTO}/bio/bss_file.c
    ${CRYPTO}/bio/bss_log.c
    ${CRYPTO}/bio/bss_mem.c
    ${CRYPTO}/bio/bss_null.c

    ${CRYPTO}/blake2/blake2b.c
    ${CRYPTO}/blake2/blake2s.c
    ${CRYPTO}/blake2/m_blake2b.c
    ${CRYPTO}/blake2/m_blake2s.c

    ${CRYPTO}/bn/bn_add.c
    ${CRYPTO}/bn/bn_asm.c
    ${CRYPTO}/bn/bn_blind.c
    ${CRYPTO}/bn/bn_const.c
    ${CRYPTO}/bn/bn_ctx.c
    ${CRYPTO}/bn/bn_div.c
    ${CRYPTO}/bn/bn_err.c
    ${CRYPTO}/bn/bn_exp.c
    ${CRYPTO}/bn/bn_exp2.c
    ${CRYPTO}/bn/bn_gcd.c
    ${CRYPTO}/bn/bn_gf2m.c
    ${CRYPTO}/bn/bn_intern.c
    ${CRYPTO}/bn/bn_kron.c
    ${CRYPTO}/bn/bn_lib.c
    ${CRYPTO}/bn/bn_mod.c
    ${CRYPTO}/bn/bn_mont.c
    ${CRYPTO}/bn/bn_mpi.c
    ${CRYPTO}/bn/bn_mul.c
    ${CRYPTO}/bn/bn_nist.c
    ${CRYPTO}/bn/bn_prime.c
    ${CRYPTO}/bn/bn_print.c
    ${CRYPTO}/bn/bn_rand.c
    ${CRYPTO}/bn/bn_recp.c
    ${CRYPTO}/bn/bn_shift.c
    ${CRYPTO}/bn/bn_sqr.c
    ${CRYPTO}/bn/bn_sqrt.c
    ${CRYPTO}/bn/bn_word.c

    ${CRYPTO}/buffer/buf_err.c
    ${CRYPTO}/buffer/buffer.c
    
    ${CRYPTO}/chacha/chacha_enc.c

    ${CRYPTO}/comp/comp_err.c
    ${CRYPTO}/comp/comp_lib.c
    ${CRYPTO}/comp/c_zlib.c

    ${CRYPTO}/conf/conf_api.c
    ${CRYPTO}/conf/conf_def.c
    ${CRYPTO}/conf/conf_err.c
    ${CRYPTO}/conf/conf_lib.c
    ${CRYPTO}/conf/conf_mall.c
    ${CRYPTO}/conf/conf_mod.c
    ${CRYPTO}/conf/conf_sap.c
    ${CRYPTO}/conf/conf_ssl.c    

    ${CRYPTO}/ct/ct_b64.c
    ${CRYPTO}/ct/ct_err.c
    ${CRYPTO}/ct/ct_log.c
    ${CRYPTO}/ct/ct_oct.c
    ${CRYPTO}/ct/ct_policy.c
    ${CRYPTO}/ct/ct_prn.c
    ${CRYPTO}/ct/ct_sct.c
    ${CRYPTO}/ct/ct_sct_ctx.c
    ${CRYPTO}/ct/ct_vfy.c
    ${CRYPTO}/ct/ct_x509v3.c

    #${CRYPTO}/dso/dso_beos.c
    ${CRYPTO}/dso/dso_dl.c
    ${CRYPTO}/dso/dso_dlfcn.c
    ${CRYPTO}/dso/dso_err.c
    ${CRYPTO}/dso/dso_lib.c
    ${CRYPTO}/dso/dso_openssl.c

    ${CRYPTO}/err/err.c
    ${CRYPTO}/err/err_all.c
    ${CRYPTO}/err/err_prn.c

    ${CRYPTO}/evp/bio_b64.c
    ${CRYPTO}/evp/bio_enc.c
    ${CRYPTO}/evp/bio_md.c
    ${CRYPTO}/evp/bio_ok.c
    ${CRYPTO}/evp/c_allc.c
    ${CRYPTO}/evp/c_alld.c
    ${CRYPTO}/evp/digest.c
    ${CRYPTO}/evp/e_aes.c
    ${CRYPTO}/evp/e_aes_cbc_hmac_sha1.c
    ${CRYPTO}/evp/e_aes_cbc_hmac_sha256.c
    ${CRYPTO}/evp/e_aria.c
    ${CRYPTO}/evp/e_chacha20_poly1305.c
    ${CRYPTO}/evp/e_null.c
    ${CRYPTO}/evp/e_old.c
#    ${CRYPTO}/evp/e_rc4_hmac_md5.c OPENSSL_NO_RC4
    ${CRYPTO}/evp/encode.c
    ${CRYPTO}/evp/evp_cnf.c
    ${CRYPTO}/evp/evp_enc.c
    ${CRYPTO}/evp/evp_err.c
    ${CRYPTO}/evp/evp_key.c
    ${CRYPTO}/evp/evp_lib.c
    ${CRYPTO}/evp/evp_pbe.c
    ${CRYPTO}/evp/evp_pkey.c
    ${CRYPTO}/evp/e_sm4.c
    ${CRYPTO}/evp/m_md5.c
    ${CRYPTO}/evp/m_md5_sha1.c
    ${CRYPTO}/evp/m_null.c
    ${CRYPTO}/evp/m_ripemd.c
    ${CRYPTO}/evp/m_sha1.c
    ${CRYPTO}/evp/m_sha3.c
    ${CRYPTO}/evp/m_sigver.c
#    ${CRYPTO}/evp/m_wp.c OPENSSL_NO_WHIRLPOOL
    ${CRYPTO}/evp/names.c
    ${CRYPTO}/evp/p5_crpt.c
    ${CRYPTO}/evp/p5_crpt2.c
    ${CRYPTO}/evp/p_dec.c
    ${CRYPTO}/evp/p_enc.c
    ${CRYPTO}/evp/p_lib.c
    ${CRYPTO}/evp/p_open.c
    ${CRYPTO}/evp/p_seal.c
    ${CRYPTO}/evp/p_sign.c
    ${CRYPTO}/evp/p_verify.c
    ${CRYPTO}/evp/pbe_scrypt.c
    ${CRYPTO}/evp/pmeth_fn.c
    ${CRYPTO}/evp/pmeth_gn.c
    ${CRYPTO}/evp/pmeth_lib.c

    ${CRYPTO}/hmac/hm_ameth.c
    ${CRYPTO}/hmac/hm_pmeth.c
    ${CRYPTO}/hmac/hmac.c

    ${CRYPTO}/kdf/kdf_err.c
    ${CRYPTO}/kdf/hkdf.c
    ${CRYPTO}/kdf/scrypt.c
    ${CRYPTO}/kdf/tls1_prf.c

    ${CRYPTO}/lhash/lh_stats.c
    ${CRYPTO}/lhash/lhash.c

    ${CRYPTO}/md5/md5_dgst.c
    ${CRYPTO}/md5/md5_one.c

    ${CRYPTO}/modes/cbc128.c
    ${CRYPTO}/modes/ccm128.c
    ${CRYPTO}/modes/cfb128.c
    ${CRYPTO}/modes/ctr128.c
    ${CRYPTO}/modes/cts128.c
    ${CRYPTO}/modes/gcm128.c
    ${CRYPTO}/modes/ocb128.c
    ${CRYPTO}/modes/ofb128.c
    ${CRYPTO}/modes/wrap128.c
    ${CRYPTO}/modes/xts128.c

    ${CRYPTO}/objects/o_names.c
    ${CRYPTO}/objects/obj_dat.c
    ${CRYPTO}/objects/obj_err.c
    ${CRYPTO}/objects/obj_lib.c
    ${CRYPTO}/objects/obj_xref.c

    ${CRYPTO}/ocsp/ocsp_asn.c
    ${CRYPTO}/ocsp/ocsp_cl.c
    ${CRYPTO}/ocsp/ocsp_err.c
    ${CRYPTO}/ocsp/ocsp_ext.c
    ${CRYPTO}/ocsp/ocsp_ht.c
    ${CRYPTO}/ocsp/ocsp_lib.c
    ${CRYPTO}/ocsp/ocsp_prn.c
    ${CRYPTO}/ocsp/ocsp_srv.c
    ${CRYPTO}/ocsp/ocsp_vfy.c
    ${CRYPTO}/ocsp/v3_ocsp.c

    ${CRYPTO}/pem/pem_all.c
    ${CRYPTO}/pem/pem_err.c
    ${CRYPTO}/pem/pem_info.c
    ${CRYPTO}/pem/pem_lib.c
    ${CRYPTO}/pem/pem_oth.c
    ${CRYPTO}/pem/pem_pk8.c
    ${CRYPTO}/pem/pem_pkey.c
    ${CRYPTO}/pem/pem_sign.c
    ${CRYPTO}/pem/pem_x509.c
    ${CRYPTO}/pem/pem_xaux.c

    ${CRYPTO}/pkcs12/p12_add.c
    ${CRYPTO}/pkcs12/p12_asn.c
    ${CRYPTO}/pkcs12/p12_attr.c
    ${CRYPTO}/pkcs12/p12_crpt.c
    ${CRYPTO}/pkcs12/p12_crt.c
    ${CRYPTO}/pkcs12/p12_decr.c
    ${CRYPTO}/pkcs12/p12_init.c
    ${CRYPTO}/pkcs12/p12_key.c
    ${CRYPTO}/pkcs12/p12_kiss.c
    ${CRYPTO}/pkcs12/p12_mutl.c
    ${CRYPTO}/pkcs12/p12_npas.c
    ${CRYPTO}/pkcs12/p12_p8d.c
    ${CRYPTO}/pkcs12/p12_p8e.c
    ${CRYPTO}/pkcs12/p12_utl.c
    ${CRYPTO}/pkcs12/pk12err.c

    ${CRYPTO}/pkcs7/bio_pk7.c
    ${CRYPTO}/pkcs7/pk7_asn1.c
    ${CRYPTO}/pkcs7/pk7_attr.c
    ${CRYPTO}/pkcs7/pk7_doit.c
    ${CRYPTO}/pkcs7/pk7_lib.c
    ${CRYPTO}/pkcs7/pk7_mime.c
    ${CRYPTO}/pkcs7/pk7_smime.c
    ${CRYPTO}/pkcs7/pkcs7err.c

    ${CRYPTO}/poly1305/poly1305.c
    ${CRYPTO}/poly1305/poly1305_pmeth.c
    ${CRYPTO}/poly1305/poly1305_ameth.c

    ${CRYPTO}/rand/drbg_ctr.c 
    ${CRYPTO}/rand/drbg_lib.c
    ${CRYPTO}/rand/rand_egd.c
    ${CRYPTO}/rand/rand_err.c
    ${CRYPTO}/rand/rand_lib.c
    ${CRYPTO}/rand/randfile.c

    ${CRYPTO}/ripemd/rmd_dgst.c
    ${CRYPTO}/ripemd/rmd_one.c

    ${CRYPTO}/rsa/rsa_ameth.c
    ${CRYPTO}/rsa/rsa_asn1.c
    ${CRYPTO}/rsa/rsa_chk.c
    ${CRYPTO}/rsa/rsa_crpt.c
    ${CRYPTO}/rsa/rsa_depr.c
    ${CRYPTO}/rsa/rsa_err.c
    ${CRYPTO}/rsa/rsa_gen.c
    ${CRYPTO}/rsa/rsa_lib.c
    ${CRYPTO}/rsa/rsa_meth.c
    ${CRYPTO}/rsa/rsa_mp.c
    ${CRYPTO}/rsa/rsa_none.c
    ${CRYPTO}/rsa/rsa_oaep.c
    ${CRYPTO}/rsa/rsa_ossl.c
    ${CRYPTO}/rsa/rsa_pk1.c
    ${CRYPTO}/rsa/rsa_pmeth.c
    ${CRYPTO}/rsa/rsa_prn.c
    ${CRYPTO}/rsa/rsa_pss.c
    ${CRYPTO}/rsa/rsa_saos.c
    ${CRYPTO}/rsa/rsa_sign.c
    ${CRYPTO}/rsa/rsa_ssl.c
    ${CRYPTO}/rsa/rsa_x931.c
    ${CRYPTO}/rsa/rsa_x931g.c

    ${CRYPTO}/siphash/siphash.c
    ${CRYPTO}/siphash/siphash_ameth.c
    ${CRYPTO}/siphash/siphash_pmeth.c

    ${CRYPTO}/sm3/sm3.c
    ${CRYPTO}/sm3/m_sm3.c

    ${CRYPTO}/sm4/sm4.c

    ${CRYPTO}/sha/sha1_one.c
    ${CRYPTO}/sha/sha1dgst.c
    ${CRYPTO}/sha/sha256.c
    ${CRYPTO}/sha/sha512.c
    ${CRYPTO}/sha/keccak1600.c
    
    ${CRYPTO}/stack/stack.c

    ${CRYPTO}/store/store_err.c
    ${CRYPTO}/store/store_init.c
    ${CRYPTO}/store/store_register.c

    ${CRYPTO}/ts/ts_asn1.c
    ${CRYPTO}/ts/ts_conf.c
    ${CRYPTO}/ts/ts_err.c
    ${CRYPTO}/ts/ts_lib.c
    ${CRYPTO}/ts/ts_req_print.c
    ${CRYPTO}/ts/ts_req_utils.c
    ${CRYPTO}/ts/ts_rsp_print.c
    ${CRYPTO}/ts/ts_rsp_sign.c
    ${CRYPTO}/ts/ts_rsp_utils.c
    ${CRYPTO}/ts/ts_rsp_verify.c
    ${CRYPTO}/ts/ts_verify_ctx.c

    ${CRYPTO}/ui/ui_err.c
    ${CRYPTO}/ui/ui_lib.c
    ${CRYPTO}/ui/ui_null.c
    ${CRYPTO}/ui/ui_openssl.c
    ${CRYPTO}/ui/ui_util.c

    ${CRYPTO}/x509/by_dir.c
    ${CRYPTO}/x509/by_file.c
    ${CRYPTO}/x509/t_x509.c
    ${CRYPTO}/x509/x509_att.c
    ${CRYPTO}/x509/x_attrib.c
    ${CRYPTO}/x509/x509_cmp.c
    ${CRYPTO}/x509/x509_d2.c
    ${CRYPTO}/x509/x509_def.c
    ${CRYPTO}/x509/x509_err.c
    ${CRYPTO}/x509/x509_ext.c
    ${CRYPTO}/x509/x509_lu.c
    ${CRYPTO}/x509/x509_obj.c
    ${CRYPTO}/x509/x509_r2x.c
    ${CRYPTO}/x509/x509_req.c
    ${CRYPTO}/x509/x509_set.c
    ${CRYPTO}/x509/x509_trs.c
    ${CRYPTO}/x509/x509_txt.c
    ${CRYPTO}/x509/x509_v3.c
    ${CRYPTO}/x509/x509_vfy.c
    ${CRYPTO}/x509/x509_vpm.c
    ${CRYPTO}/x509/x509cset.c
    ${CRYPTO}/x509/x509name.c
    ${CRYPTO}/x509/x509rset.c
    ${CRYPTO}/x509/x509spki.c
    ${CRYPTO}/x509/x509type.c
    ${CRYPTO}/x509/x_all.c
    ${CRYPTO}/x509/x_crl.c
    ${CRYPTO}/x509/x_name.c
    ${CRYPTO}/x509/x_pubkey.c
    ${CRYPTO}/x509/t_crl.c
    ${CRYPTO}/x509/t_req.c
    ${CRYPTO}/x509/x509_meth.c
    ${CRYPTO}/x509/x_exten.c
    ${CRYPTO}/x509/x_req.c
    ${CRYPTO}/x509/x_x509.c
    ${CRYPTO}/x509/x_x509a.c 

    ${CRYPTO}/x509v3/pcy_cache.c
    ${CRYPTO}/x509v3/pcy_data.c
    ${CRYPTO}/x509v3/pcy_lib.c
    ${CRYPTO}/x509v3/pcy_map.c
    ${CRYPTO}/x509v3/pcy_node.c
    ${CRYPTO}/x509v3/pcy_tree.c
    ${CRYPTO}/x509v3/v3_admis.c
    ${CRYPTO}/x509v3/v3_akey.c
    ${CRYPTO}/x509v3/v3_akeya.c
    ${CRYPTO}/x509v3/v3_alt.c
    ${CRYPTO}/x509v3/v3_bcons.c
    ${CRYPTO}/x509v3/v3_bitst.c
    ${CRYPTO}/x509v3/v3_conf.c
    ${CRYPTO}/x509v3/v3_cpols.c
    ${CRYPTO}/x509v3/v3_crld.c
    ${CRYPTO}/x509v3/v3_enum.c
    ${CRYPTO}/x509v3/v3_extku.c
    ${CRYPTO}/x509v3/v3_genn.c
    ${CRYPTO}/x509v3/v3_ia5.c
    ${CRYPTO}/x509v3/v3_info.c
    ${CRYPTO}/x509v3/v3_int.c
    ${CRYPTO}/x509v3/v3_lib.c
    ${CRYPTO}/x509v3/v3_ncons.c
    ${CRYPTO}/x509v3/v3_pci.c
    ${CRYPTO}/x509v3/v3_pcia.c
    ${CRYPTO}/x509v3/v3_pcons.c
    ${CRYPTO}/x509v3/v3_pku.c
    ${CRYPTO}/x509v3/v3_pmaps.c
    ${CRYPTO}/x509v3/v3_prn.c
    ${CRYPTO}/x509v3/v3_purp.c
    ${CRYPTO}/x509v3/v3_skey.c
    ${CRYPTO}/x509v3/v3_sxnet.c
    ${CRYPTO}/x509v3/v3_tlsf.c
    ${CRYPTO}/x509v3/v3_utl.c
    ${CRYPTO}/x509v3/v3err.c
)

if(WIN32)
    list(APPEND XSRC
    ${CRYPTO}/async/arch/async_win.c
    ${CRYPTO}/rand/rand_win.c
    )
else()
    list(APPEND XSRC
    ${CRYPTO}/async/arch/async_posix.c
    ${CRYPTO}/rand/rand_unix.c
    )
endif()  

set( CMAKE_THREAD_PREFER_PTHREAD TRUE )
find_package ( Threads )
if( NOT Threads_FOUND )
   list(APPEND XSRC
   ${CRYPTO}/threads_none.c 
   )
elseif( WIN32 )
   list(APPEND XSRC
   ${CRYPTO}/threads_win.c 
   )
else()
  list(APPEND XSRC
  ${CRYPTO}/threads_pthread.c 
  )
endif()

###########################################################################################
# INC
###########################################################################################
list(APPEND XINC
    ${OPENSSL_INC}/cryptoerr.h
    ${OPENSSL_INC}/crypto.h    
    ${OPENSSL_INC}/opensslv.h    
    ${OPENSSL_INC}/symhacks.h
    ${OPENSSL_INC}/ossl_typ.h
    ${OPENSSL_INC}/aes.h
    ${OPENSSL_INC}/asn1.h
    ${OPENSSL_INC}/asn1_mac.h
    ${OPENSSL_INC}/asn1t.h
    ${OPENSSL_INC}/bioerr.h
    ${OPENSSL_INC}/bio.h
    ${OPENSSL_INC}/bn.h
    ${OPENSSL_INC}/buffer.h
    ${OPENSSL_INC}/comp.h
    ${OPENSSL_INC}/conf.h
    ${OPENSSL_INC}/conf_api.h
    ${OPENSSL_INC}/des.h
    #${CRYPTO}/des/des_old.h
    ${INTERNAL_INC}/dso.h
    ${OPENSSL_INC}/engine.h
    ${OPENSSL_INC}/err.h
    ${OPENSSL_INC}/evp.h
    ${OPENSSL_INC}/hmac.h
    ${OPENSSL_INC}/lhash.h
    ${OPENSSL_INC}/md4.h
    ${OPENSSL_INC}/md5.h
    ${OPENSSL_INC}/modes.h
    ${OPENSSL_INC}/objects.h
    ${OPENSSL_INC}/obj_mac.h
    ${OPENSSL_INC}/ocsp.h
    ${OPENSSL_INC}/pem.h
    ${OPENSSL_INC}/pem2.h
    ${OPENSSL_INC}/pkcs12.h
    ${OPENSSL_INC}/pkcs7.h
    #${CRYPTO}/pqueue/pqueue.h
    ${OPENSSL_INC}/rand.h
    ${OPENSSL_INC}/rc5.h
    ${OPENSSL_INC}/rsa.h
    ${OPENSSL_INC}/seed.h
    ${OPENSSL_INC}/sha.h
    ${OPENSSL_INC}/stack.h
    ${OPENSSL_INC}/safestack.h
    ${OPENSSL_INC}/ts.h
    ${OPENSSL_INC}/ui.h
    #${CRYPTO}/ui/ui_compat.h
    ${OPENSSL_INC}/x509.h
    ${OPENSSL_INC}/x509_vfy.h
    ${OPENSSL_INC}/x509v3.h
    ${CRYPTO_INC}/sm4.h
)
