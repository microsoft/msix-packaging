#ifndef OSSL_CRYPTO_BN_CONF_H
# define OSSL_CRYPTO_BN_CONF_H

/*
 * The contents of this file are not used in the UEFI build, as
 * both 32-bit and 64-bit builds are supported from a single run
 * of the Configure script.
 */

/* Should we define BN_DIV2W here? */

/* Only one for the following should be defined */
#cmakedefine SIXTY_FOUR_BIT_LONG
#cmakedefine SIXTY_FOUR_BIT
#cmakedefine THIRTY_TWO_BIT

#endif
