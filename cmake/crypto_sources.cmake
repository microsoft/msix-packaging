# kudos to https://github.com/madwax/3ndparty.cmake.openssl/ for the starting point!
set( XRT "${TARGET_SOURCE_DIR_TRUE}/" )

# engines
IF ((AOSP) OR (LINUX))
	list( APPEND XSRC 
	${XRT}../engines/e_afalg_err.c    
	${XRT}../engines/e_afalg.c        
	)
ENDIF()

list( APPEND XSRC 
${XRT}../engines/e_capi_err.c     
${XRT}../engines/e_capi.c         
${XRT}../engines/e_dasync_err.c   
${XRT}../engines/e_dasync.c       
${XRT}../engines/e_ossltest_err.c
${XRT}../engines/e_ossltest.c
${XRT}../engines/e_padlock.c
)

# aes
list( APPEND XSRC 
${XRT}aes/aes_cbc.c     ${XRT}aes/aes_ecb.c     ${XRT}aes/aes_ofb.c
${XRT}aes/aes_cfb.c     ${XRT}aes/aes_ige.c     ${XRT}aes/aes_wrap.c
${XRT}aes/aes_core.c    ${XRT}aes/aes_misc.c    ${XRT}aes/aes_x86core.c
)

# asn1
list( APPEND XSRC 
${XRT}asn1/a_bitstr.c           ${XRT}asn1/a_utctm.c            ${XRT}asn1/d2i_pu.c             ${XRT}asn1/tasn_enc.c
${XRT}asn1/a_d2i_fp.c           ${XRT}asn1/a_utf8.c             ${XRT}asn1/evp_asn1.c           ${XRT}asn1/tasn_fre.c
${XRT}asn1/a_digest.c           ${XRT}asn1/a_verify.c           ${XRT}asn1/f_int.c              ${XRT}asn1/tasn_new.c
${XRT}asn1/a_dup.c              ${XRT}asn1/ameth_lib.c          ${XRT}asn1/f_string.c           ${XRT}asn1/tasn_prn.c
${XRT}asn1/a_gentm.c            ${XRT}asn1/asn1_err.c           ${XRT}asn1/i2d_pr.c             ${XRT}asn1/tasn_scn.c
${XRT}asn1/a_i2d_fp.c           ${XRT}asn1/asn1_gen.c           ${XRT}asn1/i2d_pu.c             ${XRT}asn1/tasn_typ.c
${XRT}asn1/a_int.c              ${XRT}asn1/asn1_item_list.c     ${XRT}asn1/n_pkey.c             ${XRT}asn1/tasn_utl.c
${XRT}asn1/a_mbstr.c            ${XRT}asn1/asn1_lib.c           ${XRT}asn1/nsseq.c              ${XRT}asn1/x_algor.c
${XRT}asn1/a_object.c           ${XRT}asn1/asn1_par.c           ${XRT}asn1/p5_pbe.c             ${XRT}asn1/x_bignum.c
${XRT}asn1/a_octet.c            ${XRT}asn1/asn_mime.c           ${XRT}asn1/p5_pbev2.c           ${XRT}asn1/x_info.c
${XRT}asn1/a_print.c            ${XRT}asn1/asn_moid.c           ${XRT}asn1/p5_scrypt.c          ${XRT}asn1/x_int64.c
${XRT}asn1/a_sign.c             ${XRT}asn1/asn_mstbl.c          ${XRT}asn1/p8_pkey.c            ${XRT}asn1/x_long.c
${XRT}asn1/a_strex.c            ${XRT}asn1/asn_pack.c           ${XRT}asn1/t_bitst.c            ${XRT}asn1/x_pkey.c
${XRT}asn1/a_strnid.c           ${XRT}asn1/bio_asn1.c           ${XRT}asn1/t_pkey.c             ${XRT}asn1/x_sig.c
${XRT}asn1/a_time.c             ${XRT}asn1/bio_ndef.c           ${XRT}asn1/t_spki.c             ${XRT}asn1/x_spki.c
${XRT}asn1/a_type.c             ${XRT}asn1/d2i_pr.c             ${XRT}asn1/tasn_dec.c           ${XRT}asn1/x_val.c
)

# bf
list( APPEND XSRC 
${XRT}bf/bf_cfb64.c     ${XRT}bf/bf_enc.c       ${XRT}bf/bf_skey.c
${XRT}bf/bf_ecb.c       ${XRT}bf/bf_ofb64.c
)

# bio
list( APPEND XSRC 
${XRT}bio/b_addr.c      ${XRT}bio/bf_lbuf.c     ${XRT}bio/bio_meth.c    ${XRT}bio/bss_file.c
${XRT}bio/b_dump.c      ${XRT}bio/bf_nbio.c     ${XRT}bio/bss_acpt.c    ${XRT}bio/bss_log.c
${XRT}bio/b_print.c     ${XRT}bio/bf_null.c     ${XRT}bio/bss_bio.c     ${XRT}bio/bss_mem.c
${XRT}bio/b_sock.c      ${XRT}bio/bio_cb.c      ${XRT}bio/bss_conn.c    ${XRT}bio/bss_null.c
${XRT}bio/b_sock2.c     ${XRT}bio/bio_err.c     ${XRT}bio/bss_dgram.c   ${XRT}bio/bss_sock.c
${XRT}bio/bf_buff.c     ${XRT}bio/bio_lib.c     ${XRT}bio/bss_fd.c
)

# bn
#list( APPEND XSRC 
#${XRT}bn/bn_add.c       ${XRT}bn/bn_err.c       ${XRT}bn/bn_mod.c       ${XRT}bn/bn_recp.c
#${XRT}bn/bn_asm.c       ${XRT}bn/bn_exp.c       ${XRT}bn/bn_mont.c      ${XRT}bn/bn_shift.c
#${XRT}bn/bn_blind.c     ${XRT}bn/bn_exp2.c      ${XRT}bn/bn_mpi.c       ${XRT}bn/bn_sqr.c
#${XRT}bn/bn_const.c     ${XRT}bn/bn_gcd.c       ${XRT}bn/bn_mul.c       ${XRT}bn/bn_sqrt.c
#${XRT}bn/bn_ctx.c       ${XRT}bn/bn_gf2m.c      ${XRT}bn/bn_nist.c      ${XRT}bn/bn_srp.c
#${XRT}bn/bn_depr.c      ${XRT}bn/bn_intern.c    ${XRT}bn/bn_prime.c     ${XRT}bn/bn_word.c
#${XRT}bn/bn_dh.c        ${XRT}bn/bn_kron.c      ${XRT}bn/bn_print.c     ${XRT}bn/bn_x931p.c
#${XRT}bn/bn_div.c       ${XRT}bn/bn_lib.c       ${XRT}bn/bn_rand.c      ${XRT}bn/rsaz_exp.c
#)

# buffer
list( APPEND XSRC 
${XRT}buffer/buf_err.c  ${XRT}buffer/buffer.c
)

# camella
list( APPEND XSRC 
${XRT}camellia/camellia.c       ${XRT}camellia/cmll_cfb.c       ${XRT}camellia/cmll_ecb.c       ${XRT}camellia/cmll_ofb.c
${XRT}camellia/cmll_cbc.c       ${XRT}camellia/cmll_ctr.c       ${XRT}camellia/cmll_misc.c
)

# cast
list( APPEND XSRC 
${XRT}cast/c_cfb64.c    ${XRT}cast/c_enc.c      ${XRT}cast/c_skey.c
${XRT}cast/c_ecb.c      ${XRT}cast/c_ofb64.c
)

# cmac
list( APPEND XSRC 
${XRT}cmac/cm_ameth.c   ${XRT}cmac/cm_pmeth.c   ${XRT}cmac/cmac.c
)

# cms
list( APPEND XSRC 
${XRT}cms/cms_asn1.c    ${XRT}cms/cms_enc.c     ${XRT}cms/cms_io.c      ${XRT}cms/cms_sd.c
${XRT}cms/cms_att.c     ${XRT}cms/cms_env.c     ${XRT}cms/cms_kari.c    ${XRT}cms/cms_smime.c
${XRT}cms/cms_cd.c      ${XRT}cms/cms_err.c     ${XRT}cms/cms_lib.c
${XRT}cms/cms_dd.c      ${XRT}cms/cms_ess.c     ${XRT}cms/cms_pwri.c
)

#comp
list( APPEND XSRC 
${XRT}comp/c_zlib.c     ${XRT}comp/comp_err.c   ${XRT}comp/comp_lib.c
)

# conf
list( APPEND XSRC 
${XRT}conf/conf_api.c   ${XRT}conf/conf_err.c   ${XRT}conf/conf_mall.c  ${XRT}conf/conf_sap.c
${XRT}conf/conf_def.c   ${XRT}conf/conf_lib.c   ${XRT}conf/conf_mod.c
)

# des
list( APPEND XSRC 
${XRT}des/cbc_cksm.c    ${XRT}des/des_enc.c     ${XRT}des/ncbc_enc.c    ${XRT}des/qud_cksm.c
${XRT}des/cbc_enc.c     ${XRT}des/ecb3_enc.c    ${XRT}des/ofb64ede.c    ${XRT}des/rand_key.c
${XRT}des/cfb64ede.c    ${XRT}des/ecb_enc.c     ${XRT}des/ofb64enc.c    ${XRT}des/set_key.c
${XRT}des/cfb64enc.c    ${XRT}des/fcrypt.c      ${XRT}des/ofb_enc.c     ${XRT}des/str2key.c
${XRT}des/cfb_enc.c     ${XRT}des/fcrypt_b.c    ${XRT}des/pcbc_enc.c    ${XRT}des/xcbc_enc.c
)

# dh
list( APPEND XSRC 
${XRT}dh/dh_ameth.c     ${XRT}dh/dh_err.c       ${XRT}dh/dh_lib.c       ${XRT}dh/dh_rfc5114.c
${XRT}dh/dh_asn1.c      ${XRT}dh/dh_gen.c       ${XRT}dh/dh_meth.c      ${XRT}dh/dh_rfc7919.c
${XRT}dh/dh_check.c     ${XRT}dh/dh_kdf.c       ${XRT}dh/dh_pmeth.c
${XRT}dh/dh_depr.c      ${XRT}dh/dh_key.c       ${XRT}dh/dh_prn.c
)

# dsa
list( APPEND XSRC 
${XRT}dsa/dsa_ameth.c   ${XRT}dsa/dsa_gen.c     ${XRT}dsa/dsa_ossl.c    ${XRT}dsa/dsa_vrf.c
${XRT}dsa/dsa_asn1.c    ${XRT}dsa/dsa_key.c     ${XRT}dsa/dsa_pmeth.c
${XRT}dsa/dsa_depr.c    ${XRT}dsa/dsa_lib.c     ${XRT}dsa/dsa_prn.c
${XRT}dsa/dsa_err.c     ${XRT}dsa/dsa_meth.c    ${XRT}dsa/dsa_sign.c
)

# dso
#list( APPEND XSRC 
#${XRT}dso/dso_dl.c      ${XRT}dso/dso_err.c     ${XRT}dso/dso_openssl.c ${XRT}dso/dso_win32.c
#${XRT}dso/dso_dlfcn.c   ${XRT}dso/dso_lib.c     ${XRT}dso/dso_vms.c
#)

# ec
#list( APPEND XSRC 
#${XRT}ec/curve25519.c           ${XRT}ec/ec_err.c               ${XRT}ec/ecdh_ossl.c            ${XRT}ec/ecp_nistp521.c
#${XRT}ec/ec2_mult.c             ${XRT}ec/ec_key.c               ${XRT}ec/ecdsa_ossl.c           ${XRT}ec/ecp_nistputil.c
#${XRT}ec/ec2_oct.c              ${XRT}ec/ec_kmeth.c             ${XRT}ec/ecdsa_sign.c           ${XRT}ec/ecp_nistz256.c
#${XRT}ec/ec2_smpl.c             ${XRT}ec/ec_lib.c               ${XRT}ec/ecdsa_vrf.c            ${XRT}ec/ecp_nistz256_table.c
#${XRT}ec/ec_ameth.c             ${XRT}ec/ec_mult.c              ${XRT}ec/eck_prn.c              ${XRT}ec/ecp_oct.c
#${XRT}ec/ec_asn1.c              ${XRT}ec/ec_oct.c               ${XRT}ec/ecp_mont.c             ${XRT}ec/ecp_smpl.c
#${XRT}ec/ec_check.c             ${XRT}ec/ec_pmeth.c             ${XRT}ec/ecp_nist.c             ${XRT}ec/ecx_meth.c
#${XRT}ec/ec_curve.c             ${XRT}ec/ec_print.c             ${XRT}ec/ecp_nistp224.c
#${XRT}ec/ec_cvt.c               ${XRT}ec/ecdh_kdf.c             ${XRT}ec/ecp_nistp256.c
#)

# engine
list( APPEND XSRC 
${XRT}engine/eng_all.c          ${XRT}engine/eng_fat.c          ${XRT}engine/eng_rdrand.c       ${XRT}engine/tb_dsa.c
${XRT}engine/eng_cnf.c          ${XRT}engine/eng_init.c         ${XRT}engine/eng_table.c        ${XRT}engine/tb_eckey.c
${XRT}engine/eng_ctrl.c         ${XRT}engine/eng_lib.c          ${XRT}engine/tb_asnmth.c        ${XRT}engine/tb_pkmeth.c

#${XRT}engine/eng_devcrypto.c    ${XRT}engine/eng_list.c         
																${XRT}engine/tb_cipher.c        ${XRT}engine/tb_rand.c
${XRT}engine/eng_dyn.c          ${XRT}engine/eng_openssl.c      ${XRT}engine/tb_dh.c            ${XRT}engine/tb_rsa.c
${XRT}engine/eng_err.c          ${XRT}engine/eng_pkey.c         ${XRT}engine/tb_digest.c
)

# err
list( APPEND XSRC 
${XRT}err/err.c         ${XRT}err/err_all.c     ${XRT}err/err_prn.c
)

# evp
list( APPEND XSRC 
${XRT}evp/bio_b64.c                     ${XRT}evp/e_rc2.c                       ${XRT}evp/m_ripemd.c
${XRT}evp/bio_enc.c                     ${XRT}evp/e_rc4.c                       ${XRT}evp/m_sha1.c
${XRT}evp/bio_md.c                      ${XRT}evp/e_rc4_hmac_md5.c              ${XRT}evp/m_sha3.c
${XRT}evp/bio_ok.c                      ${XRT}evp/e_rc5.c                       ${XRT}evp/m_sigver.c
${XRT}evp/c_allc.c                      ${XRT}evp/e_seed.c                      ${XRT}evp/m_wp.c
${XRT}evp/c_alld.c                      ${XRT}evp/e_sm4.c                       ${XRT}evp/names.c
${XRT}evp/cmeth_lib.c                   ${XRT}evp/e_xcbc_d.c                    ${XRT}evp/p5_crpt.c
${XRT}evp/digest.c                      ${XRT}evp/encode.c                      ${XRT}evp/p5_crpt2.c
${XRT}evp/e_aes.c                       ${XRT}evp/evp_cnf.c                     ${XRT}evp/p_dec.c
${XRT}evp/e_aes_cbc_hmac_sha1.c         ${XRT}evp/evp_enc.c                     ${XRT}evp/p_enc.c
${XRT}evp/e_aes_cbc_hmac_sha256.c       ${XRT}evp/evp_err.c                     ${XRT}evp/p_lib.c
${XRT}evp/e_aria.c                      ${XRT}evp/evp_key.c                     ${XRT}evp/p_open.c
${XRT}evp/e_bf.c                        ${XRT}evp/evp_lib.c                     ${XRT}evp/p_seal.c
${XRT}evp/e_camellia.c                  ${XRT}evp/evp_pbe.c                     ${XRT}evp/p_sign.c
${XRT}evp/e_cast.c                      ${XRT}evp/evp_pkey.c                    ${XRT}evp/p_verify.c
${XRT}evp/e_chacha20_poly1305.c         ${XRT}evp/m_md2.c                       ${XRT}evp/pbe_scrypt.c
${XRT}evp/e_des.c                       ${XRT}evp/m_md4.c                       ${XRT}evp/pmeth_fn.c
${XRT}evp/e_des3.c                      ${XRT}evp/m_md5.c                       ${XRT}evp/pmeth_gn.c
${XRT}evp/e_idea.c                      ${XRT}evp/m_md5_sha1.c                  ${XRT}evp/pmeth_lib.c
${XRT}evp/e_null.c                      ${XRT}evp/m_mdc2.c
${XRT}evp/e_old.c                       ${XRT}evp/m_null.c
)

# hmac
list( APPEND XSRC 
${XRT}hmac/hm_ameth.c   ${XRT}hmac/hm_pmeth.c   ${XRT}hmac/hmac.c
)

# idea
list( APPEND XSRC 
${XRT}idea/i_cbc.c      ${XRT}idea/i_ecb.c      ${XRT}idea/i_skey.c
${XRT}idea/i_cfb64.c    ${XRT}idea/i_ofb64.c
)

# lhash
list( APPEND XSRC 
${XRT}lhash/lh_stats.c  ${XRT}lhash/lhash.c
)

# md4
list( APPEND XSRC 
${XRT}md4/md4_dgst.c    ${XRT}md4/md4_one.c
)

# md5
list( APPEND XSRC 
${XRT}md5/md5_dgst.c    ${XRT}md5/md5_one.c
)

# mdc2
list( APPEND XSRC 
${XRT}mdc2/mdc2_one.c   ${XRT}mdc2/mdc2dgst.c
)

# modes  
list( APPEND XSRC 
${XRT}modes/cbc128.c    ${XRT}modes/ctr128.c    ${XRT}modes/ocb128.c    ${XRT}modes/xts128.c
${XRT}modes/ccm128.c    ${XRT}modes/cts128.c    ${XRT}modes/ofb128.c
${XRT}modes/cfb128.c    ${XRT}modes/gcm128.c    ${XRT}modes/wrap128.c
)

# objects
list( APPEND XSRC 
${XRT}objects/o_names.c         ${XRT}objects/obj_err.c         ${XRT}objects/obj_xref.c
${XRT}objects/obj_dat.c         ${XRT}objects/obj_lib.c
)

# ocsp
list( APPEND XSRC 
${XRT}ocsp/ocsp_asn.c   ${XRT}ocsp/ocsp_ext.c   ${XRT}ocsp/ocsp_prn.c   ${XRT}ocsp/v3_ocsp.c
${XRT}ocsp/ocsp_cl.c    ${XRT}ocsp/ocsp_ht.c    ${XRT}ocsp/ocsp_srv.c
${XRT}ocsp/ocsp_err.c   ${XRT}ocsp/ocsp_lib.c   ${XRT}ocsp/ocsp_vfy.c
)

# pem
list( APPEND XSRC 
${XRT}pem/pem_all.c     ${XRT}pem/pem_lib.c     ${XRT}pem/pem_pkey.c    ${XRT}pem/pem_xaux.c
${XRT}pem/pem_err.c     ${XRT}pem/pem_oth.c     ${XRT}pem/pem_sign.c    ${XRT}pem/pvkfmt.c
${XRT}pem/pem_info.c    ${XRT}pem/pem_pk8.c     ${XRT}pem/pem_x509.c
)

# pkcs12
list( APPEND XSRC 
${XRT}pkcs12/p12_add.c  ${XRT}pkcs12/p12_crt.c  ${XRT}pkcs12/p12_kiss.c ${XRT}pkcs12/p12_p8e.c
${XRT}pkcs12/p12_asn.c  ${XRT}pkcs12/p12_decr.c ${XRT}pkcs12/p12_mutl.c ${XRT}pkcs12/p12_sbag.c
${XRT}pkcs12/p12_attr.c ${XRT}pkcs12/p12_init.c ${XRT}pkcs12/p12_npas.c ${XRT}pkcs12/p12_utl.c
${XRT}pkcs12/p12_crpt.c ${XRT}pkcs12/p12_key.c  ${XRT}pkcs12/p12_p8d.c  ${XRT}pkcs12/pk12err.c
)

# pkcs7
list( APPEND XSRC 
${XRT}pkcs7/bio_pk7.c   ${XRT}pkcs7/pk7_attr.c  ${XRT}pkcs7/pk7_lib.c   ${XRT}pkcs7/pk7_smime.c
${XRT}pkcs7/pk7_asn1.c  ${XRT}pkcs7/pk7_doit.c  ${XRT}pkcs7/pk7_mime.c  ${XRT}pkcs7/pkcs7err.c
)

# rand
list( APPEND XSRC 
${XRT}rand/drbg_lib.c   ${XRT}rand/rand_err.c   ${XRT}rand/rand_vms.c
${XRT}rand/drbg_rand.c  ${XRT}rand/rand_lib.c   ${XRT}rand/rand_win.c
${XRT}rand/rand_egd.c   ${XRT}rand/rand_unix.c  ${XRT}rand/randfile.c
)

# rc2
list( APPEND XSRC 
${XRT}rc2/rc2_cbc.c     ${XRT}rc2/rc2_skey.c    ${XRT}rc2/rc2ofb64.c
${XRT}rc2/rc2_ecb.c     ${XRT}rc2/rc2cfb64.c
)

# rc4
list( APPEND XSRC 
${XRT}rc4/rc4_enc.c     ${XRT}rc4/rc4_skey.c
)

# rc5
list( APPEND XSRC 
${XRT}rc5/rc5_ecb.c     ${XRT}rc5/rc5_skey.c    ${XRT}rc5/rc5ofb64.c
${XRT}rc5/rc5_enc.c     ${XRT}rc5/rc5cfb64.c
)

# ripemd
list( APPEND XSRC 
${XRT}ripemd/rmd_dgst.c ${XRT}ripemd/rmd_one.c
)

# rsa
list( APPEND XSRC 
${XRT}rsa/rsa_ameth.c   ${XRT}rsa/rsa_gen.c     ${XRT}rsa/rsa_pk1.c     ${XRT}rsa/rsa_ssl.c
${XRT}rsa/rsa_asn1.c    ${XRT}rsa/rsa_lib.c     ${XRT}rsa/rsa_pmeth.c   ${XRT}rsa/rsa_x931.c
${XRT}rsa/rsa_chk.c     ${XRT}rsa/rsa_meth.c    ${XRT}rsa/rsa_prn.c     ${XRT}rsa/rsa_x931g.c
${XRT}rsa/rsa_crpt.c    ${XRT}rsa/rsa_none.c    ${XRT}rsa/rsa_pss.c
${XRT}rsa/rsa_depr.c    ${XRT}rsa/rsa_oaep.c    ${XRT}rsa/rsa_saos.c
${XRT}rsa/rsa_err.c     ${XRT}rsa/rsa_ossl.c    ${XRT}rsa/rsa_sign.c
)

# seed
list( APPEND XSRC 
${XRT}seed/seed.c       ${XRT}seed/seed_cfb.c   ${XRT}seed/seed_ofb.c
${XRT}seed/seed_cbc.c   ${XRT}seed/seed_ecb.c
)

# sha
list( APPEND XSRC 
${XRT}sha/keccak1600.c  ${XRT}sha/sha1dgst.c    ${XRT}sha/sha512.c
${XRT}sha/sha1_one.c    ${XRT}sha/sha256.c
)

# srp
list( APPEND XSRC 
${XRT}srp/srp_lib.c     ${XRT}srp/srp_vfy.c
)

# stack
list( APPEND XSRC 
${XRT}stack/stack.c
)

# ts
list( APPEND XSRC 
${XRT}ts/ts_asn1.c              ${XRT}ts/ts_lib.c               ${XRT}ts/ts_rsp_print.c         ${XRT}ts/ts_rsp_verify.c
${XRT}ts/ts_conf.c              ${XRT}ts/ts_req_print.c         ${XRT}ts/ts_rsp_sign.c          ${XRT}ts/ts_verify_ctx.c
${XRT}ts/ts_err.c               ${XRT}ts/ts_req_utils.c         ${XRT}ts/ts_rsp_utils.c
)

# txt_db
list( APPEND XSRC 
${XRT}txt_db/txt_db.c
)

# ui
list( APPEND XSRC 
${XRT}ui/ui_err.c       ${XRT}ui/ui_null.c      ${XRT}ui/ui_util.c
${XRT}ui/ui_lib.c       ${XRT}ui/ui_openssl.c
)

list( APPEND XSRC 
${XRT}uid.c
)

# whrlpool
list( APPEND XSRC 
${XRT}whrlpool/wp_block.c       ${XRT}whrlpool/wp_dgst.c
)

# x509
list( APPEND XSRC 
${XRT}x509/by_dir.c     ${XRT}x509/x509_err.c   ${XRT}x509/x509_v3.c    ${XRT}x509/x_attrib.c
${XRT}x509/by_file.c    ${XRT}x509/x509_ext.c   ${XRT}x509/x509_vfy.c   ${XRT}x509/x_crl.c
${XRT}x509/t_crl.c      ${XRT}x509/x509_lu.c    ${XRT}x509/x509_vpm.c   ${XRT}x509/x_exten.c
${XRT}x509/t_req.c      ${XRT}x509/x509_obj.c   ${XRT}x509/x509cset.c   ${XRT}x509/x_name.c
${XRT}x509/t_x509.c     ${XRT}x509/x509_r2x.c   ${XRT}x509/x509name.c   ${XRT}x509/x_pubkey.c
${XRT}x509/x509_att.c   ${XRT}x509/x509_req.c   ${XRT}x509/x509rset.c   ${XRT}x509/x_req.c
${XRT}x509/x509_cmp.c   ${XRT}x509/x509_set.c   ${XRT}x509/x509spki.c   ${XRT}x509/x_x509.c
${XRT}x509/x509_d2.c    ${XRT}x509/x509_trs.c   ${XRT}x509/x509type.c   ${XRT}x509/x_x509a.c
${XRT}x509/x509_def.c   ${XRT}x509/x509_txt.c   ${XRT}x509/x_all.c
)

# x509v3
list( APPEND XSRC 
${XRT}x509v3/pcy_cache.c        ${XRT}x509v3/v3_alt.c           ${XRT}x509v3/v3_ia5.c           ${XRT}x509v3/v3_prn.c
${XRT}x509v3/pcy_data.c         ${XRT}x509v3/v3_asid.c          ${XRT}x509v3/v3_info.c          ${XRT}x509v3/v3_purp.c
${XRT}x509v3/pcy_lib.c          ${XRT}x509v3/v3_bcons.c         ${XRT}x509v3/v3_int.c           ${XRT}x509v3/v3_skey.c
${XRT}x509v3/pcy_map.c          ${XRT}x509v3/v3_bitst.c         ${XRT}x509v3/v3_lib.c           ${XRT}x509v3/v3_sxnet.c
${XRT}x509v3/pcy_node.c         ${XRT}x509v3/v3_conf.c          ${XRT}x509v3/v3_ncons.c         ${XRT}x509v3/v3_tlsf.c
${XRT}x509v3/pcy_tree.c         ${XRT}x509v3/v3_cpols.c         ${XRT}x509v3/v3_pci.c           ${XRT}x509v3/v3_utl.c
${XRT}x509v3/v3_addr.c          ${XRT}x509v3/v3_crld.c          ${XRT}x509v3/v3_pcia.c          ${XRT}x509v3/v3err.c
${XRT}x509v3/v3_admis.c         ${XRT}x509v3/v3_enum.c          ${XRT}x509v3/v3_pcons.c
${XRT}x509v3/v3_akey.c          ${XRT}x509v3/v3_extku.c         ${XRT}x509v3/v3_pku.c
${XRT}x509v3/v3_akeya.c         ${XRT}x509v3/v3_genn.c          ${XRT}x509v3/v3_pmaps.c
)

list( APPEND XSRC 
${XRT}LPdir_nyi.c       ${XRT}cryptlib.c        ${XRT}mem_clr.c         ${XRT}o_time.c
${XRT}LPdir_unix.c      ${XRT}ctype.c           ${XRT}mem_dbg.c         ${XRT}ppccap.c
${XRT}LPdir_vms.c       ${XRT}cversion.c        ${XRT}mem_sec.c         ${XRT}s390xcap.c
${XRT}LPdir_win.c       ${XRT}dllmain.c         ${XRT}o_dir.c           ${XRT}sparcv9cap.c
${XRT}LPdir_win32.c     ${XRT}ebcdic.c          ${XRT}o_fips.c          ${XRT}threads_none.c
${XRT}LPdir_wince.c     ${XRT}ex_data.c         ${XRT}o_fopen.c         ${XRT}threads_pthread.c
${XRT}armcap.c          ${XRT}init.c            ${XRT}o_init.c          ${XRT}threads_win.c
${XRT}cpt_err.c         ${XRT}mem.c             ${XRT}o_str.c           ${XRT}uid.c
)

###########################################################################################
# INC
###########################################################################################
set( PUBL "${OpenSSL_SOURCE_PATH}/include/openssl/")
set( AES  "${OpenSSL_SOURCE_PATH}/crypto/aes/")
set( BIO  "${OpenSSL_SOURCE_PATH}/crypto/bio/")
list( APPEND XINC 
${AES}aes.h             #${PUBL}ct.h
								                ${PUBL}md5.h            ${PUBL}seed.h
${PUBL}asn1.h           ${PUBL}cterr.h          ${PUBL}mdc2.h           ${PUBL}sha.h
${PUBL}asn1_mac.h       ${PUBL}des.h            ${PUBL}modes.h          ${PUBL}srp.h
${PUBL}asn1err.h        ${PUBL}dh.h             ${PUBL}obj_mac.h        ${PUBL}srtp.h
${PUBL}asn1t.h          ${PUBL}dherr.h          ${PUBL}objects.h        ${PUBL}ssl.h
${PUBL}async.h          ${PUBL}dsa.h            ${PUBL}objectserr.h     ${PUBL}ssl2.h
${PUBL}asyncerr.h       ${PUBL}dsaerr.h         ${PUBL}ocsp.h           ${PUBL}ssl3.h
${BIO}bio.h             ${PUBL}dtls1.h          ${PUBL}ocsperr.h        ${PUBL}sslerr.h
${PUBL}bioerr.h         ${PUBL}e_os2.h          ${PUBL}opensslv.h       ${PUBL}stack.h
${PUBL}blowfish.h       ${PUBL}ebcdic.h         ${PUBL}ossl_typ.h       ${PUBL}store.h
${PUBL}bn.h             ${PUBL}ec.h             ${PUBL}pem.h            ${PUBL}storeerr.h
${PUBL}bnerr.h          ${PUBL}ecdh.h           ${PUBL}pem2.h           ${PUBL}symhacks.h
${PUBL}buffer.h         ${PUBL}ecdsa.h          ${PUBL}pemerr.h         ${PUBL}tls1.h
${PUBL}buffererr.h      ${PUBL}ecerr.h          ${PUBL}pkcs12.h         ${PUBL}ts.h
${PUBL}camellia.h       ${PUBL}engine.h         ${PUBL}pkcs12err.h      ${PUBL}tserr.h
${PUBL}cast.h           ${PUBL}engineerr.h      ${PUBL}pkcs7.h          ${PUBL}txt_db.h
${PUBL}cmac.h           ${PUBL}err.h            ${PUBL}pkcs7err.h       ${PUBL}ui.h
${PUBL}cms.h            ${PUBL}evp.h            ${PUBL}rand.h           ${PUBL}uierr.h
${PUBL}cmserr.h         ${PUBL}evperr.h         ${PUBL}randerr.h        ${PUBL}whrlpool.h
${PUBL}comp.h           ${PUBL}hmac.h           ${PUBL}rc2.h            ${PUBL}x509.h
${PUBL}comperr.h        ${PUBL}idea.h           ${PUBL}rc4.h            ${PUBL}x509_vfy.h
${PUBL}conf.h           ${PUBL}kdf.h            ${PUBL}rc5.h            ${PUBL}x509err.h
${PUBL}conf_api.h       ${PUBL}kdferr.h         ${PUBL}ripemd.h         ${PUBL}x509v3.h
${PUBL}conferr.h        ${PUBL}lhash.h          ${PUBL}rsa.h            ${PUBL}x509v3err.h
${PUBL}crypto.h         ${PUBL}md2.h            ${PUBL}rsaerr.h
${PUBL}cryptoerr.h      ${PUBL}md4.h            ${PUBL}safestack.h
)

set( PRIV "${OpenSSL_SOURCE_PATH}/include/internal/")
list( APPEND XPNC 
${PRIV}/bio.h                   ${PRIV}/dso.h                   ${PRIV}/o_str.h
${PRIV}/comp.h                  ${PRIV}/dsoerr.h                ${PRIV}/rand.h
${PRIV}/conf.h                  ${PRIV}/err.h                   ${PRIV}/refcount.h
${PRIV}/constant_time_locl.h    ${PRIV}/nelem.h                 ${PRIV}/sockets.h
${PRIV}/cryptlib.h              ${PRIV}/numbers.h               ${PRIV}/thread_once.h
${PRIV}/dane.h                  ${PRIV}/o_dir.h
)

set (CPRV "${OpenSSL_SOURCE_PATH}/crypto/include/internal/")
list( APPEND XPNC 
${CPRV}aria.h           ${CPRV}ctype.h          ${CPRV}siphash.h
${CPRV}asn1_int.h       ${CPRV}engine.h         ${CPRV}sm3.h
${CPRV}async.h          ${CPRV}err_int.h        ${CPRV}sm4.h
${CPRV}bn_dh.h          ${CPRV}evp_int.h        ${CPRV}store.h
${CPRV}bn_int.h         ${CPRV}md32_common.h    ${CPRV}store_int.h
${CPRV}bn_srp.h         ${CPRV}objects.h        ${CPRV}x509_int.h
${CPRV}chacha.h         ${CPRV}poly1305.h
${CPRV}cryptlib_int.h   ${CPRV}rand_int.h
)