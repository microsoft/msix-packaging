/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 1



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     MYEOF = 0,
     _LANG_XPATH2_ = 258,
     _LANG_XQUERY_ = 259,
     _LANG_XSLT2_ = 260,
     _LANG_FUNCDECL_ = 261,
     _LANG_DELAYEDMODULE_ = 262,
     _LANG_FUNCTION_SIGNATURE_ = 263,
     _DOLLAR_ = 264,
     _COLON_EQUALS_ = 265,
     _ASTERISK_ = 266,
     _BAR_ = 267,
     _BANG_EQUALS_ = 268,
     _LESS_THAN_EQUALS_ = 269,
     _GREATER_THAN_ = 270,
     _GREATER_THAN_EQUALS_ = 271,
     _LESS_THAN_LESS_THAN_ = 272,
     _GREATER_THAN_GREATER_THAN_ = 273,
     _COLON_COLON_ = 274,
     _AT_SIGN_ = 275,
     _LSQUARE_ = 276,
     _RSQUARE_ = 277,
     _QUESTION_MARK_ = 278,
     _LESS_THAN_OP_OR_TAG_ = 279,
     _START_TAG_CLOSE_ = 280,
     _END_TAG_CLOSE_ = 281,
     _PI_START_ = 282,
     _XML_COMMENT_START_ = 283,
     _XML_COMMENT_END_ = 284,
     _SLASH_SLASH_ = 285,
     _END_TAG_OPEN_ = 286,
     _PRAGMA_OPEN_ = 287,
     _COMMA_ = 288,
     _PLUS_ = 289,
     _MINUS_ = 290,
     _SLASH_ = 291,
     _EQUALS_ = 292,
     _DOT_ = 293,
     _DOT_DOT_ = 294,
     _LPAR_ = 295,
     _RPAR_ = 296,
     _EMPTY_TAG_CLOSE_ = 297,
     _VALUE_INDICATOR_ = 298,
     _OPEN_QUOT_ = 299,
     _CLOSE_QUOT_ = 300,
     _OPEN_APOS_ = 301,
     _CLOSE_APOS_ = 302,
     _LBRACE_ = 303,
     _RBRACE_ = 304,
     _SEMICOLON_ = 305,
     _HASH_ = 306,
     _INTEGER_LITERAL_ = 307,
     _DECIMAL_LITERAL_ = 308,
     _DOUBLE_LITERAL_ = 309,
     _ATTRIBUTE_ = 310,
     _COMMENT_ = 311,
     _DOCUMENT_NODE_ = 312,
     _NAMESPACE_NODE_ = 313,
     _NODE_ = 314,
     _PROCESSING_INSTRUCTION_ = 315,
     _SCHEMA_ATTRIBUTE_ = 316,
     _SCHEMA_ELEMENT_ = 317,
     _TEXT_ = 318,
     _EMPTY_SEQUENCE_ = 319,
     _BOUNDARY_SPACE_ = 320,
     _FT_OPTION_ = 321,
     _BASE_URI_ = 322,
     _LAX_ = 323,
     _STRICT_ = 324,
     _IDIV_ = 325,
     _CHILD_ = 326,
     _DESCENDANT_ = 327,
     _SELF_ = 328,
     _DESCENDANT_OR_SELF_ = 329,
     _FOLLOWING_SIBLING_ = 330,
     _FOLLOWING_ = 331,
     _PARENT_ = 332,
     _ANCESTOR_ = 333,
     _PRECEDING_SIBLING_ = 334,
     _PRECEDING_ = 335,
     _ANCESTOR_OR_SELF_ = 336,
     _DOCUMENT_ = 337,
     _NOT_ = 338,
     _USING_ = 339,
     _NO_ = 340,
     _SENSITIVE_ = 341,
     _INSENSITIVE_ = 342,
     _DIACRITICS_ = 343,
     _WITHOUT_ = 344,
     _WITHOUT_C_ = 345,
     _STEMMING_ = 346,
     _THESAURUS_ = 347,
     _STOP_ = 348,
     _WILDCARDS_ = 349,
     _ENTIRE_ = 350,
     _CONTENT_ = 351,
     _WORD_ = 352,
     _TYPE_ = 353,
     _START_ = 354,
     _END_ = 355,
     _MOST_ = 356,
     _SKIP_ = 357,
     _COPY_ = 358,
     _VALUE_ = 359,
     _WHITESPACE_ = 360,
     _PI_CONTENT_ = 361,
     _XML_COMMENT_CONTENT_ = 362,
     _EQ_ = 363,
     _NE_ = 364,
     _LT_ = 365,
     _LE_ = 366,
     _GT_ = 367,
     _GE_ = 368,
     _AT_ = 369,
     _QUOT_ATTR_CONTENT_ = 370,
     _APOS_ATTR_CONTENT_ = 371,
     _WHITESPACE_ELEMENT_CONTENT_ = 372,
     _ELEMENT_CONTENT_ = 373,
     _AT_LM_ = 374,
     _ORDERED_ = 375,
     _UNORDERED_ = 376,
     _QNAME_ = 377,
     _XMLNS_QNAME_ = 378,
     _CONSTR_QNAME_ = 379,
     _STRING_LITERAL_ = 380,
     _VARIABLE_ = 381,
     _NCNAME_COLON_STAR_ = 382,
     _STAR_COLON_NCNAME_ = 383,
     _PI_TARGET_ = 384,
     _PRAGMA_CONTENT_ = 385,
     _RETURN_ = 386,
     _FOR_ = 387,
     _IN_ = 388,
     _LET_ = 389,
     _WHERE_ = 390,
     _COUNT_ = 391,
     _BY_ = 392,
     _ORDER_ = 393,
     _STABLE_ = 394,
     _ASCENDING_ = 395,
     _DESCENDING_ = 396,
     _EMPTY_ = 397,
     _GREATEST_ = 398,
     _LEAST_ = 399,
     _COLLATION_ = 400,
     _SOME_ = 401,
     _EVERY_ = 402,
     _SATISFIES_ = 403,
     _TYPESWITCH_ = 404,
     _CASE_ = 405,
     _CASE_S_ = 406,
     _AS_ = 407,
     _IF_ = 408,
     _THEN_ = 409,
     _ELSE_ = 410,
     _OR_ = 411,
     _AND_ = 412,
     _INSTANCE_ = 413,
     _OF_ = 414,
     _CASTABLE_ = 415,
     _TO_ = 416,
     _DIV_ = 417,
     _MOD_ = 418,
     _UNION_ = 419,
     _INTERSECT_ = 420,
     _EXCEPT_ = 421,
     _VALIDATE_ = 422,
     _CAST_ = 423,
     _TREAT_ = 424,
     _IS_ = 425,
     _PRESERVE_ = 426,
     _STRIP_ = 427,
     _NAMESPACE_ = 428,
     _ITEM_ = 429,
     _EXTERNAL_ = 430,
     _ENCODING_ = 431,
     _NO_PRESERVE_ = 432,
     _INHERIT_ = 433,
     _NO_INHERIT_ = 434,
     _DECLARE_ = 435,
     _CONSTRUCTION_ = 436,
     _ORDERING_ = 437,
     _DEFAULT_ = 438,
     _COPY_NAMESPACES_ = 439,
     _OPTION_ = 440,
     _XQUERY_ = 441,
     _VERSION_ = 442,
     _IMPORT_ = 443,
     _SCHEMA_ = 444,
     _MODULE_ = 445,
     _ELEMENT_ = 446,
     _FUNCTION_ = 447,
     _FUNCTION_EXT_ = 448,
     _SCORE_ = 449,
     _CONTAINS_ = 450,
     _WEIGHT_ = 451,
     _WINDOW_ = 452,
     _DISTANCE_ = 453,
     _OCCURS_ = 454,
     _TIMES_ = 455,
     _SAME_ = 456,
     _DIFFERENT_ = 457,
     _LOWERCASE_ = 458,
     _UPPERCASE_ = 459,
     _RELATIONSHIP_ = 460,
     _LEVELS_ = 461,
     _LANGUAGE_ = 462,
     _ANY_ = 463,
     _ALL_ = 464,
     _PHRASE_ = 465,
     _EXACTLY_ = 466,
     _FROM_ = 467,
     _WORDS_ = 468,
     _SENTENCES_ = 469,
     _PARAGRAPHS_ = 470,
     _SENTENCE_ = 471,
     _PARAGRAPH_ = 472,
     _REPLACE_ = 473,
     _MODIFY_ = 474,
     _FIRST_ = 475,
     _INSERT_ = 476,
     _BEFORE_ = 477,
     _AFTER_ = 478,
     _REVALIDATION_ = 479,
     _WITH_ = 480,
     _NODES_ = 481,
     _RENAME_ = 482,
     _LAST_ = 483,
     _DELETE_ = 484,
     _INTO_ = 485,
     _UPDATING_ = 486,
     _ID_ = 487,
     _KEY_ = 488,
     _TEMPLATE_ = 489,
     _MATCHES_ = 490,
     _NAME_ = 491,
     _CALL_ = 492,
     _APPLY_ = 493,
     _TEMPLATES_ = 494,
     _MODE_ = 495,
     _FTOR_ = 496,
     _FTAND_ = 497,
     _FTNOT_ = 498,
     _PRIVATE_ = 499,
     _PUBLIC_ = 500,
     _DETERMINISTIC_ = 501,
     _NONDETERMINISTIC_ = 502,
     _XSLT_END_ELEMENT_ = 503,
     _XSLT_STYLESHEET_ = 504,
     _XSLT_TEMPLATE_ = 505,
     _XSLT_VALUE_OF_ = 506,
     _XSLT_TEXT_ = 507,
     _XSLT_APPLY_TEMPLATES_ = 508,
     _XSLT_CALL_TEMPLATE_ = 509,
     _XSLT_WITH_PARAM_ = 510,
     _XSLT_SEQUENCE_ = 511,
     _XSLT_PARAM_ = 512,
     _XSLT_FUNCTION_ = 513,
     _XSLT_CHOOSE_ = 514,
     _XSLT_WHEN_ = 515,
     _XSLT_OTHERWISE_ = 516,
     _XSLT_IF_ = 517,
     _XSLT_VARIABLE_ = 518,
     _XSLT_COMMENT_ = 519,
     _XSLT_PI_ = 520,
     _XSLT_DOCUMENT_ = 521,
     _XSLT_ATTRIBUTE_ = 522,
     _XSLT_NAMESPACE_ = 523,
     _XSLT_ELEMENT_ = 524,
     _XSLT_ANALYZE_STRING_ = 525,
     _XSLT_MATCHING_SUBSTRING_ = 526,
     _XSLT_NON_MATCHING_SUBSTRING_ = 527,
     _XSLT_COPY_OF_ = 528,
     _XSLT_COPY_ = 529,
     _XSLT_FOR_EACH_ = 530,
     _XSLT_OUTPUT_ = 531,
     _XSLT_IMPORT_SCHEMA_ = 532,
     _XSLT_VERSION_ = 533,
     _XSLT_MODE_ = 534,
     _XSLT_NAME_ = 535,
     _XSLT_DOCTYPE_PUBLIC_ = 536,
     _XSLT_DOCTYPE_SYSTEM_ = 537,
     _XSLT_ENCODING_ = 538,
     _XSLT_MEDIA_TYPE_ = 539,
     _XSLT_NORMALIZATION_FORM_ = 540,
     _XSLT_STANDALONE_ = 541,
     _XSLT_EXCLUDE_RESULT_PREFIXES_ = 542,
     _XSLT_NAMESPACE_STR_ = 543,
     _XSLT_SCHEMA_LOCATION_ = 544,
     _XSLT_TUNNEL_ = 545,
     _XSLT_REQUIRED_ = 546,
     _XSLT_OVERRIDE_ = 547,
     _XSLT_COPY_NAMESPACES_ = 548,
     _XSLT_INHERIT_NAMESPACES_ = 549,
     _XSLT_BYTE_ORDER_MARK_ = 550,
     _XSLT_ESCAPE_URI_ATTRIBUTES_ = 551,
     _XSLT_INCLUDE_CONTENT_TYPE_ = 552,
     _XSLT_INDENT_ = 553,
     _XSLT_OMIT_XML_DECLARATION_ = 554,
     _XSLT_UNDECLARE_PREFIXES_ = 555,
     _XSLT_MATCH_ = 556,
     _XSLT_AS_ = 557,
     _XSLT_SELECT_ = 558,
     _XSLT_PRIORITY_ = 559,
     _XSLT_TEST_ = 560,
     _XSLT_SEPARATOR_ = 561,
     _XSLT_NAMESPACE_A_ = 562,
     _XSLT_REGEX_ = 563,
     _XSLT_FLAGS_ = 564,
     _XSLT_METHOD_ = 565,
     _XSLT_CDATA_SECTION_ELEMENTS_ = 566,
     _XSLT_USE_CHARACTER_MAPS_ = 567,
     _XSLT_ELEMENT_NAME_ = 568,
     _XSLT_XMLNS_ATTR_ = 569,
     _XSLT_ATTR_NAME_ = 570,
     _XSLT_TEXT_NODE_ = 571,
     _XSLT_WS_TEXT_NODE_ = 572,
     _HASH_DEFAULT_ = 573,
     _HASH_ALL_ = 574,
     _HASH_CURRENT_ = 575,
     _XML_ = 576,
     _HTML_ = 577,
     _XHTML_ = 578
   };
#endif
/* Tokens.  */
#define MYEOF 0
#define _LANG_XPATH2_ 258
#define _LANG_XQUERY_ 259
#define _LANG_XSLT2_ 260
#define _LANG_FUNCDECL_ 261
#define _LANG_DELAYEDMODULE_ 262
#define _LANG_FUNCTION_SIGNATURE_ 263
#define _DOLLAR_ 264
#define _COLON_EQUALS_ 265
#define _ASTERISK_ 266
#define _BAR_ 267
#define _BANG_EQUALS_ 268
#define _LESS_THAN_EQUALS_ 269
#define _GREATER_THAN_ 270
#define _GREATER_THAN_EQUALS_ 271
#define _LESS_THAN_LESS_THAN_ 272
#define _GREATER_THAN_GREATER_THAN_ 273
#define _COLON_COLON_ 274
#define _AT_SIGN_ 275
#define _LSQUARE_ 276
#define _RSQUARE_ 277
#define _QUESTION_MARK_ 278
#define _LESS_THAN_OP_OR_TAG_ 279
#define _START_TAG_CLOSE_ 280
#define _END_TAG_CLOSE_ 281
#define _PI_START_ 282
#define _XML_COMMENT_START_ 283
#define _XML_COMMENT_END_ 284
#define _SLASH_SLASH_ 285
#define _END_TAG_OPEN_ 286
#define _PRAGMA_OPEN_ 287
#define _COMMA_ 288
#define _PLUS_ 289
#define _MINUS_ 290
#define _SLASH_ 291
#define _EQUALS_ 292
#define _DOT_ 293
#define _DOT_DOT_ 294
#define _LPAR_ 295
#define _RPAR_ 296
#define _EMPTY_TAG_CLOSE_ 297
#define _VALUE_INDICATOR_ 298
#define _OPEN_QUOT_ 299
#define _CLOSE_QUOT_ 300
#define _OPEN_APOS_ 301
#define _CLOSE_APOS_ 302
#define _LBRACE_ 303
#define _RBRACE_ 304
#define _SEMICOLON_ 305
#define _HASH_ 306
#define _INTEGER_LITERAL_ 307
#define _DECIMAL_LITERAL_ 308
#define _DOUBLE_LITERAL_ 309
#define _ATTRIBUTE_ 310
#define _COMMENT_ 311
#define _DOCUMENT_NODE_ 312
#define _NAMESPACE_NODE_ 313
#define _NODE_ 314
#define _PROCESSING_INSTRUCTION_ 315
#define _SCHEMA_ATTRIBUTE_ 316
#define _SCHEMA_ELEMENT_ 317
#define _TEXT_ 318
#define _EMPTY_SEQUENCE_ 319
#define _BOUNDARY_SPACE_ 320
#define _FT_OPTION_ 321
#define _BASE_URI_ 322
#define _LAX_ 323
#define _STRICT_ 324
#define _IDIV_ 325
#define _CHILD_ 326
#define _DESCENDANT_ 327
#define _SELF_ 328
#define _DESCENDANT_OR_SELF_ 329
#define _FOLLOWING_SIBLING_ 330
#define _FOLLOWING_ 331
#define _PARENT_ 332
#define _ANCESTOR_ 333
#define _PRECEDING_SIBLING_ 334
#define _PRECEDING_ 335
#define _ANCESTOR_OR_SELF_ 336
#define _DOCUMENT_ 337
#define _NOT_ 338
#define _USING_ 339
#define _NO_ 340
#define _SENSITIVE_ 341
#define _INSENSITIVE_ 342
#define _DIACRITICS_ 343
#define _WITHOUT_ 344
#define _WITHOUT_C_ 345
#define _STEMMING_ 346
#define _THESAURUS_ 347
#define _STOP_ 348
#define _WILDCARDS_ 349
#define _ENTIRE_ 350
#define _CONTENT_ 351
#define _WORD_ 352
#define _TYPE_ 353
#define _START_ 354
#define _END_ 355
#define _MOST_ 356
#define _SKIP_ 357
#define _COPY_ 358
#define _VALUE_ 359
#define _WHITESPACE_ 360
#define _PI_CONTENT_ 361
#define _XML_COMMENT_CONTENT_ 362
#define _EQ_ 363
#define _NE_ 364
#define _LT_ 365
#define _LE_ 366
#define _GT_ 367
#define _GE_ 368
#define _AT_ 369
#define _QUOT_ATTR_CONTENT_ 370
#define _APOS_ATTR_CONTENT_ 371
#define _WHITESPACE_ELEMENT_CONTENT_ 372
#define _ELEMENT_CONTENT_ 373
#define _AT_LM_ 374
#define _ORDERED_ 375
#define _UNORDERED_ 376
#define _QNAME_ 377
#define _XMLNS_QNAME_ 378
#define _CONSTR_QNAME_ 379
#define _STRING_LITERAL_ 380
#define _VARIABLE_ 381
#define _NCNAME_COLON_STAR_ 382
#define _STAR_COLON_NCNAME_ 383
#define _PI_TARGET_ 384
#define _PRAGMA_CONTENT_ 385
#define _RETURN_ 386
#define _FOR_ 387
#define _IN_ 388
#define _LET_ 389
#define _WHERE_ 390
#define _COUNT_ 391
#define _BY_ 392
#define _ORDER_ 393
#define _STABLE_ 394
#define _ASCENDING_ 395
#define _DESCENDING_ 396
#define _EMPTY_ 397
#define _GREATEST_ 398
#define _LEAST_ 399
#define _COLLATION_ 400
#define _SOME_ 401
#define _EVERY_ 402
#define _SATISFIES_ 403
#define _TYPESWITCH_ 404
#define _CASE_ 405
#define _CASE_S_ 406
#define _AS_ 407
#define _IF_ 408
#define _THEN_ 409
#define _ELSE_ 410
#define _OR_ 411
#define _AND_ 412
#define _INSTANCE_ 413
#define _OF_ 414
#define _CASTABLE_ 415
#define _TO_ 416
#define _DIV_ 417
#define _MOD_ 418
#define _UNION_ 419
#define _INTERSECT_ 420
#define _EXCEPT_ 421
#define _VALIDATE_ 422
#define _CAST_ 423
#define _TREAT_ 424
#define _IS_ 425
#define _PRESERVE_ 426
#define _STRIP_ 427
#define _NAMESPACE_ 428
#define _ITEM_ 429
#define _EXTERNAL_ 430
#define _ENCODING_ 431
#define _NO_PRESERVE_ 432
#define _INHERIT_ 433
#define _NO_INHERIT_ 434
#define _DECLARE_ 435
#define _CONSTRUCTION_ 436
#define _ORDERING_ 437
#define _DEFAULT_ 438
#define _COPY_NAMESPACES_ 439
#define _OPTION_ 440
#define _XQUERY_ 441
#define _VERSION_ 442
#define _IMPORT_ 443
#define _SCHEMA_ 444
#define _MODULE_ 445
#define _ELEMENT_ 446
#define _FUNCTION_ 447
#define _FUNCTION_EXT_ 448
#define _SCORE_ 449
#define _CONTAINS_ 450
#define _WEIGHT_ 451
#define _WINDOW_ 452
#define _DISTANCE_ 453
#define _OCCURS_ 454
#define _TIMES_ 455
#define _SAME_ 456
#define _DIFFERENT_ 457
#define _LOWERCASE_ 458
#define _UPPERCASE_ 459
#define _RELATIONSHIP_ 460
#define _LEVELS_ 461
#define _LANGUAGE_ 462
#define _ANY_ 463
#define _ALL_ 464
#define _PHRASE_ 465
#define _EXACTLY_ 466
#define _FROM_ 467
#define _WORDS_ 468
#define _SENTENCES_ 469
#define _PARAGRAPHS_ 470
#define _SENTENCE_ 471
#define _PARAGRAPH_ 472
#define _REPLACE_ 473
#define _MODIFY_ 474
#define _FIRST_ 475
#define _INSERT_ 476
#define _BEFORE_ 477
#define _AFTER_ 478
#define _REVALIDATION_ 479
#define _WITH_ 480
#define _NODES_ 481
#define _RENAME_ 482
#define _LAST_ 483
#define _DELETE_ 484
#define _INTO_ 485
#define _UPDATING_ 486
#define _ID_ 487
#define _KEY_ 488
#define _TEMPLATE_ 489
#define _MATCHES_ 490
#define _NAME_ 491
#define _CALL_ 492
#define _APPLY_ 493
#define _TEMPLATES_ 494
#define _MODE_ 495
#define _FTOR_ 496
#define _FTAND_ 497
#define _FTNOT_ 498
#define _PRIVATE_ 499
#define _PUBLIC_ 500
#define _DETERMINISTIC_ 501
#define _NONDETERMINISTIC_ 502
#define _XSLT_END_ELEMENT_ 503
#define _XSLT_STYLESHEET_ 504
#define _XSLT_TEMPLATE_ 505
#define _XSLT_VALUE_OF_ 506
#define _XSLT_TEXT_ 507
#define _XSLT_APPLY_TEMPLATES_ 508
#define _XSLT_CALL_TEMPLATE_ 509
#define _XSLT_WITH_PARAM_ 510
#define _XSLT_SEQUENCE_ 511
#define _XSLT_PARAM_ 512
#define _XSLT_FUNCTION_ 513
#define _XSLT_CHOOSE_ 514
#define _XSLT_WHEN_ 515
#define _XSLT_OTHERWISE_ 516
#define _XSLT_IF_ 517
#define _XSLT_VARIABLE_ 518
#define _XSLT_COMMENT_ 519
#define _XSLT_PI_ 520
#define _XSLT_DOCUMENT_ 521
#define _XSLT_ATTRIBUTE_ 522
#define _XSLT_NAMESPACE_ 523
#define _XSLT_ELEMENT_ 524
#define _XSLT_ANALYZE_STRING_ 525
#define _XSLT_MATCHING_SUBSTRING_ 526
#define _XSLT_NON_MATCHING_SUBSTRING_ 527
#define _XSLT_COPY_OF_ 528
#define _XSLT_COPY_ 529
#define _XSLT_FOR_EACH_ 530
#define _XSLT_OUTPUT_ 531
#define _XSLT_IMPORT_SCHEMA_ 532
#define _XSLT_VERSION_ 533
#define _XSLT_MODE_ 534
#define _XSLT_NAME_ 535
#define _XSLT_DOCTYPE_PUBLIC_ 536
#define _XSLT_DOCTYPE_SYSTEM_ 537
#define _XSLT_ENCODING_ 538
#define _XSLT_MEDIA_TYPE_ 539
#define _XSLT_NORMALIZATION_FORM_ 540
#define _XSLT_STANDALONE_ 541
#define _XSLT_EXCLUDE_RESULT_PREFIXES_ 542
#define _XSLT_NAMESPACE_STR_ 543
#define _XSLT_SCHEMA_LOCATION_ 544
#define _XSLT_TUNNEL_ 545
#define _XSLT_REQUIRED_ 546
#define _XSLT_OVERRIDE_ 547
#define _XSLT_COPY_NAMESPACES_ 548
#define _XSLT_INHERIT_NAMESPACES_ 549
#define _XSLT_BYTE_ORDER_MARK_ 550
#define _XSLT_ESCAPE_URI_ATTRIBUTES_ 551
#define _XSLT_INCLUDE_CONTENT_TYPE_ 552
#define _XSLT_INDENT_ 553
#define _XSLT_OMIT_XML_DECLARATION_ 554
#define _XSLT_UNDECLARE_PREFIXES_ 555
#define _XSLT_MATCH_ 556
#define _XSLT_AS_ 557
#define _XSLT_SELECT_ 558
#define _XSLT_PRIORITY_ 559
#define _XSLT_TEST_ 560
#define _XSLT_SEPARATOR_ 561
#define _XSLT_NAMESPACE_A_ 562
#define _XSLT_REGEX_ 563
#define _XSLT_FLAGS_ 564
#define _XSLT_METHOD_ 565
#define _XSLT_CDATA_SECTION_ELEMENTS_ 566
#define _XSLT_USE_CHARACTER_MAPS_ 567
#define _XSLT_ELEMENT_NAME_ 568
#define _XSLT_XMLNS_ATTR_ 569
#define _XSLT_ATTR_NAME_ 570
#define _XSLT_TEXT_NODE_ 571
#define _XSLT_WS_TEXT_NODE_ 572
#define _HASH_DEFAULT_ 573
#define _HASH_ALL_ 574
#define _HASH_CURRENT_ 575
#define _XML_ 576
#define _HTML_ 577
#define _XHTML_ 578




/* Copy the first part of user declarations.  */
#line 20 "../src/parser/XQParser.y"


#if defined(WIN32) && !defined(__CYGWIN__)
// turn off the warnings "switch statement contains 'default' but no 'case' labels"
//                       "'yyerrlab1' : unreferenced label"
#pragma warning(disable : 4065 4102)
#endif

#include <cstdio>
#include <iostream>
#include <sstream>

#include <xqilla/utils/UTF8Str.hpp>

#include "../lexer/XQLexer.hpp"

#include <xqilla/simple-api/XQQuery.hpp>
#include <xqilla/ast/XQFunction.hpp>
#include <xqilla/ast/XQDocumentConstructor.hpp>
#include <xqilla/ast/XQElementConstructor.hpp>
#include <xqilla/ast/XQAttributeConstructor.hpp>
#include <xqilla/ast/XQNamespaceConstructor.hpp>
#include <xqilla/ast/XQPIConstructor.hpp>
#include <xqilla/ast/XQCommentConstructor.hpp>
#include <xqilla/ast/XQTextConstructor.hpp>
#include <xqilla/ast/XQQuantified.hpp>
#include <xqilla/ast/XQTypeswitch.hpp>
#include <xqilla/ast/XQValidate.hpp>
#include <xqilla/ast/XQGlobalVariable.hpp>
#include <xqilla/ast/XQFunctionCall.hpp>
#include <xqilla/ast/XQOrderingChange.hpp>
#include <xqilla/ast/XQDocumentOrder.hpp>
#include <xqilla/ast/XQReturn.hpp>
#include <xqilla/ast/XQNamespaceBinding.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/dom/DOMXPathNSResolver.hpp>

#include <xqilla/ast/XQSequence.hpp>
#include <xqilla/ast/XQNav.hpp>
#include <xqilla/ast/XQStep.hpp>
#include <xqilla/ast/XQLiteral.hpp>
#include <xqilla/ast/XQVariable.hpp>
#include <xqilla/ast/XQCastAs.hpp>
#include <xqilla/ast/XQCastableAs.hpp>
#include <xqilla/ast/XQTreatAs.hpp>
#include <xqilla/ast/XQFunctionConversion.hpp>
#include <xqilla/ast/XQAnalyzeString.hpp>
#include <xqilla/ast/XQCopyOf.hpp>
#include <xqilla/ast/XQCopy.hpp>
#include <xqilla/ast/XQAtomize.hpp>
#include <xqilla/ast/ConvertFunctionArg.hpp>
#include <xqilla/ast/XQIf.hpp>
#include <xqilla/ast/XQContextItem.hpp>
#include <xqilla/ast/XQPredicate.hpp>
#include <xqilla/ast/XQApplyTemplates.hpp>
#include <xqilla/ast/XQInlineFunction.hpp>
#include <xqilla/ast/XQFunctionDeref.hpp>
#include <xqilla/ast/XQFunctionRef.hpp>
#include <xqilla/ast/XQPartialApply.hpp>
#include <xqilla/ast/XQMap.hpp>

#include <xqilla/fulltext/FTContains.hpp>
#include <xqilla/fulltext/FTOr.hpp>
#include <xqilla/fulltext/FTAnd.hpp>
#include <xqilla/fulltext/FTMildnot.hpp>
#include <xqilla/fulltext/FTUnaryNot.hpp>
#include <xqilla/fulltext/FTOrder.hpp>
#include <xqilla/fulltext/FTDistance.hpp>
#include <xqilla/fulltext/FTScope.hpp>
#include <xqilla/fulltext/FTContent.hpp>
#include <xqilla/fulltext/FTWindow.hpp>

#include <xqilla/update/UDelete.hpp>
#include <xqilla/update/URename.hpp>
#include <xqilla/update/UReplace.hpp>
#include <xqilla/update/UReplaceValueOf.hpp>
#include <xqilla/update/UInsertAsFirst.hpp>
#include <xqilla/update/UInsertAsLast.hpp>
#include <xqilla/update/UInsertInto.hpp>
#include <xqilla/update/UInsertAfter.hpp>
#include <xqilla/update/UInsertBefore.hpp>
#include <xqilla/update/UTransform.hpp>
#include <xqilla/update/UApplyUpdates.hpp>

#include <xqilla/ast/ContextTuple.hpp>
#include <xqilla/ast/ForTuple.hpp>
#include <xqilla/ast/LetTuple.hpp>
#include <xqilla/ast/WhereTuple.hpp>
#include <xqilla/ast/OrderByTuple.hpp>
#include <xqilla/ast/CountTuple.hpp>

#include <xqilla/parser/QName.hpp>

#include <xqilla/operators/Equals.hpp>
#include <xqilla/operators/GeneralComp.hpp>
#include <xqilla/operators/NotEquals.hpp>
#include <xqilla/operators/LessThan.hpp>
#include <xqilla/operators/GreaterThan.hpp>
#include <xqilla/operators/LessThanEqual.hpp>
#include <xqilla/operators/GreaterThanEqual.hpp>
#include <xqilla/operators/Plus.hpp>
#include <xqilla/operators/Minus.hpp>
#include <xqilla/operators/Multiply.hpp>
#include <xqilla/operators/Divide.hpp>
#include <xqilla/operators/IntegerDivide.hpp>
#include <xqilla/operators/Mod.hpp>
#include <xqilla/operators/UnaryMinus.hpp>
#include <xqilla/operators/NodeComparison.hpp>
#include <xqilla/operators/OrderComparison.hpp>
#include <xqilla/operators/Intersect.hpp>
#include <xqilla/operators/Except.hpp>
#include <xqilla/operators/Union.hpp>
#include <xqilla/operators/Or.hpp>
#include <xqilla/operators/And.hpp>

#include <xqilla/functions/FunctionRoot.hpp>
#include <xqilla/functions/FunctionQName.hpp>
#include <xqilla/functions/FunctionId.hpp>
#include <xqilla/functions/FunctionError.hpp>
#include <xqilla/functions/XQillaFunction.hpp>
#include <xqilla/functions/BuiltInModules.hpp>

#include <xqilla/axis/NodeTest.hpp>

#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/NamespaceLookupException.hpp>
#include <xqilla/exceptions/ContextException.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>

#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/utils/UTF8Str.hpp>
#include "../config/xqilla_config.h"

#define YYPARSE_PARAM qp
#define YYDEBUG 1
#define YYERROR_VERBOSE

// this removes a memory leak occurring when an error is found in the query (we throw an exception from inside
// yyerror, preventing the bison-generated code from cleaning up the memory); by defining this macro we use 
// stack-based memory instead of the heap
#define YYSTACK_USE_ALLOCA  1
#if HAVE_ALLOCA_H
#include <alloca.h>
#elif defined __GNUC__
#undef alloca
#define alloca __builtin_alloca
#elif defined _AIX
#define alloca __alloca
#elif defined _MSC_VER
#include <malloc.h>
#else
#include <stddef.h>
#ifdef __cplusplus
extern "C"
#endif
void *alloca (size_t);
#endif

#define QP       ((XQParserArgs*)qp)
#define CONTEXT  (QP->_context)
#define LANGUAGE (QP->_lexer->getLanguage())
#define MEMMGR   (QP->_lexer->getMemoryManager())

#define REJECT_NOT_XQUERY(where,pos)      if(!QP->_lexer->isXQuery()) { yyerror(LANGUAGE, #where, (pos).first_line, (pos).first_column); }
#define REJECT_NOT_VERSION3(where,pos)   if(!QP->_lexer->isVersion3()) { yyerror(LANGUAGE, #where, (pos).first_line, (pos).first_column); }

#define WRAP(pos,object)        (wrapForDebug((QP), (object), (pos).first_line, (pos).first_column))

#define LOCATION(pos,name) LocationInfo name(QP->_lexer->getFile(), (pos).first_line, (pos).first_column)

#define GET_NAVIGATION(pos, object) (((object)->getType() == ASTNode::NAVIGATION) ? (XQNav*)(object) : WRAP(pos, new (MEMMGR) XQNav((object), MEMMGR)))

#define PRESERVE_NS(pos, object) (QP->_lexer->getNSResolver() == 0 ? (object) : WRAP((pos), new (MEMMGR) XQNamespaceBinding(QP->_lexer->getNSResolver(), (object), MEMMGR)))

#define BIT_ORDERING_SPECIFIED                  0
#define BIT_BOUNDARY_SPECIFIED                  1
#define BIT_COLLATION_SPECIFIED                 2
#define BIT_BASEURI_SPECIFIED                   3
#define BIT_CONSTRUCTION_SPECIFIED              4
#define BIT_EMPTYORDERING_SPECIFIED             5
#define BIT_COPYNAMESPACE_SPECIFIED             6
#define BIT_DEFAULTELEMENTNAMESPACE_SPECIFIED   7
#define BIT_DEFAULTFUNCTIONNAMESPACE_SPECIFIED  8
#define BIT_DECLARE_SECOND_STEP                 9
#define BIT_REVALIDATION_SPECIFIED              10

#define CHECK_SECOND_STEP(pos, name) { \
  if(QP->_flags.get(BIT_DECLARE_SECOND_STEP)) \
    yyerror((pos), "Prolog contains " name " declaration after a variable, function or option declaration"); \
}

#define CHECK_SPECIFIED(pos, bit, name, error) { \
  if(QP->_flags.get(bit)) \
    yyerror((pos), "Prolog contains more than one " name " declaration [err:" error "]"); \
  QP->_flags.set(bit); \
}

#define SET_NAMESPACE(pos, prefix, uri) { \
  if(QP->_namespaceDecls.containsKey((prefix))) \
    yyerror((pos), "Namespace prefix has already been bound to a namespace [err:XQST0033]"); \
  QP->_namespaceDecls.put((prefix), NULL); \
  CONTEXT->setNamespaceBinding((prefix), (uri)); \
}

#define SET_BUILT_IN_NAMESPACE(prefix, uri) { \
  if(!QP->_namespaceDecls.containsKey((prefix))) \
    CONTEXT->setNamespaceBinding((prefix), (uri)); \
}

#undef yylex
#define yylex QP->_lexer->yylex
#undef yyerror
#define yyerror QP->_lexer->error

XERCES_CPP_NAMESPACE_USE;
using namespace std;

static const XMLCh sz1_0[] = { chDigit_1, chPeriod, chDigit_0, chNull };
static const XMLCh sz3_0[] = { chDigit_3, chPeriod, chDigit_0, chNull };
static const XMLCh option_projection[] = { 'p', 'r', 'o', 'j', 'e', 'c', 't', 'i', 'o', 'n', 0 };
static const XMLCh option_psvi[] = { 'p', 's', 'v', 'i', 0 };
static const XMLCh option_lint[] = { 'l', 'i', 'n', 't', 0 };
static const XMLCh var_name[] = { 'n', 'a', 'm', 'e', 0 };

static const XMLCh XMLChXS[]    = { chLatin_x, chLatin_s, chNull };
static const XMLCh XMLChXSI[]   = { chLatin_x, chLatin_s, chLatin_i, chNull };
static const XMLCh XMLChFN[]    = { chLatin_f, chLatin_n, chNull };
static const XMLCh XMLChLOCAL[] = { chLatin_l, chLatin_o, chLatin_c, chLatin_a, chLatin_l, chNull };
static const XMLCh XMLChERR[]   = { chLatin_e, chLatin_r, chLatin_r, chNull };

static inline VectorOfASTNodes packageArgs(ASTNode *arg1Impl, ASTNode *arg2Impl, XPath2MemoryManager* memMgr)
{
  VectorOfASTNodes args=VectorOfASTNodes(2,(ASTNode*)NULL,XQillaAllocator<ASTNode*>(memMgr));
  args[0]=arg1Impl;
  args[1]=arg2Impl;

  return args;
}

static inline TupleNode *getLastAncestor(TupleNode *node)
{
  while(node->getParent()) node = const_cast<TupleNode*>(node->getParent());
  return node;
}

static inline TupleNode *setLastAncestor(TupleNode *node, TupleNode *ancestor)
{
  getLastAncestor(node)->setParent(ancestor);
  return node;
}

template<typename TYPE>
TYPE *wrapForDebug(XQParserArgs *qp, TYPE* pObjToWrap, unsigned int line, unsigned int column)
{
  if(pObjToWrap->getLine() == 0)
    pObjToWrap->setLocationInfo(QP->_lexer->getFile(), line, column);
  return pObjToWrap;
}

#define RESOLVE_QNAME(pos, qname) const XMLCh *uri, *name; resolveQName((pos), QP, (qname), uri, name)

static void resolveQName(const yyltype &pos, XQParserArgs *qp, const XMLCh *qname, const XMLCh *&uri, const XMLCh *&name)
{
  const XMLCh *prefix = XPath2NSUtils::getPrefix(qname, MEMMGR);
  name = XPath2NSUtils::getLocalName(qname);

  if(prefix == 0 || *prefix == 0) {
    uri = 0;
  }
  else {
    if(QP->_lexer->getNSResolver() == 0) {
      if(prefix == 0 || *prefix == 0)
        uri = CONTEXT->getDefaultElementAndTypeNS();
      else uri = CONTEXT->getNSResolver()->lookupNamespaceURI(prefix);
    }
    else {
      uri = QP->_lexer->getNSResolver()->lookupNamespaceURI(prefix);
    }

    if(uri == 0 || *uri == 0) {
      ostringstream oss;
      oss << "No namespace binding for prefix '" << UTF8(prefix) << "' [err:XTSE0280]";
      yyerror(pos, oss.str().c_str());
    }
  }
}

#define XSLT_VARIABLE_VALUE(pos, select, seqConstruct, seqType) variableValueXSLT((pos), QP, (select), (seqConstruct), (seqType))

static ASTNode *variableValueXSLT(const yyltype &pos, XQParserArgs *qp, ASTNode *select, XQSequence *seqConstruct, SequenceType *seqType)
{
  if(!seqConstruct->getChildren().empty()) {
    if(select != 0) return 0; // signifies an error
    
    if(seqType == 0) {
      return WRAP(pos, new (MEMMGR) XQDocumentConstructor(seqConstruct, MEMMGR));
    }

    return WRAP(pos, new (MEMMGR) XQFunctionConversion(seqConstruct, seqType, MEMMGR));
  }

  if(select != 0) {
    if(seqType == 0) return select;

    return WRAP(pos, new (MEMMGR) XQFunctionConversion(select, seqType, MEMMGR));
  }

  if(seqType == 0) {
    return WRAP(pos, new (MEMMGR) XQLiteral(
                  SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                  SchemaSymbols::fgDT_STRING,
                  XMLUni::fgZeroLenString, AnyAtomicType::STRING,
                  MEMMGR));
  }

  return WRAP(pos, new (MEMMGR) XQFunctionConversion(WRAP(pos, new (MEMMGR) XQSequence(MEMMGR)), seqType, MEMMGR));
}

namespace XQParser {



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 1091 "../src/parser/XQParser.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
    YYLTYPE yyls;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  21
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   7369

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  324
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  347
/* YYNRULES -- Number of rules.  */
#define YYNRULES  991
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1620

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   578

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     7,    11,    15,    18,    21,    24,    25,
      26,    35,    36,    38,    42,    45,    52,    53,    56,    65,
      67,    72,    74,    75,    78,    81,    82,    85,    88,    91,
      94,    97,   100,   105,   107,   111,   114,   118,   122,   126,
     127,   130,   133,   136,   138,   140,   142,   147,   149,   152,
     156,   159,   160,   163,   167,   169,   172,   176,   180,   183,
     186,   190,   192,   195,   199,   203,   206,   209,   213,   215,
     218,   222,   226,   229,   231,   234,   238,   241,   245,   248,
     251,   254,   257,   260,   263,   266,   269,   272,   275,   278,
     282,   285,   287,   289,   291,   293,   295,   297,   300,   303,
     305,   308,   311,   315,   319,   324,   325,   329,   332,   333,
     336,   339,   340,   343,   346,   349,   351,   353,   355,   357,
     359,   361,   363,   365,   367,   369,   371,   373,   375,   377,
     379,   381,   383,   385,   389,   391,   395,   399,   403,   405,
     407,   411,   413,   417,   421,   423,   425,   427,   428,   431,
     435,   437,   440,   441,   444,   448,   450,   453,   457,   461,
     464,   467,   471,   475,   479,   484,   486,   489,   493,   497,
     498,   502,   507,   509,   513,   517,   521,   522,   526,   527,
     531,   536,   538,   541,   545,   549,   553,   555,   559,   563,
     565,   569,   573,   577,   579,   583,   585,   589,   593,   597,
     601,   605,   607,   611,   615,   619,   621,   625,   629,   632,
     634,   638,   641,   645,   647,   650,   653,   657,   659,   663,
     667,   669,   671,   673,   675,   677,   679,   684,   691,   693,
     695,   697,   699,   701,   704,   708,   712,   716,   720,   723,
     725,   729,   733,   736,   737,   742,   745,   748,   750,   752,
     756,   761,   765,   768,   771,   774,   777,   784,   785,   789,
     793,   797,   801,   805,   809,   813,   817,   821,   823,   825,
     827,   829,   831,   833,   835,   837,   839,   841,   843,   849,
     853,   857,   863,   869,   874,   878,   882,   886,   892,   898,
     904,   906,   908,   910,   912,   917,   921,   927,   932,   933,
     936,   940,   944,   948,   956,   961,   968,   971,   973,   977,
     981,   989,   991,   993,   996,  1000,  1002,  1004,  1005,  1008,
    1011,  1014,  1016,  1018,  1020,  1022,  1023,  1026,  1030,  1032,
    1036,  1040,  1042,  1046,  1048,  1050,  1052,  1054,  1056,  1058,
    1060,  1062,  1064,  1066,  1068,  1070,  1072,  1076,  1078,  1081,
    1084,  1086,  1088,  1090,  1092,  1094,  1097,  1101,  1103,  1111,
    1112,  1116,  1117,  1121,  1124,  1128,  1130,  1136,  1142,  1145,
    1149,  1153,  1158,  1162,  1164,  1168,  1174,  1176,  1177,  1179,
    1181,  1182,  1185,  1188,  1193,  1198,  1202,  1204,  1210,  1217,
    1220,  1222,  1228,  1232,  1236,  1240,  1244,  1245,  1249,  1258,
    1262,  1264,  1268,  1270,  1274,  1278,  1279,  1284,  1288,  1292,
    1296,  1300,  1304,  1308,  1312,  1316,  1320,  1324,  1328,  1332,
    1334,  1339,  1345,  1347,  1351,  1353,  1357,  1361,  1363,  1367,
    1371,  1375,  1379,  1381,  1385,  1389,  1391,  1395,  1399,  1401,
    1406,  1408,  1413,  1415,  1420,  1422,  1427,  1429,  1432,  1435,
    1437,  1439,  1441,  1443,  1448,  1454,  1460,  1464,  1469,  1471,
    1474,  1479,  1481,  1484,  1486,  1489,  1492,  1494,  1496,  1500,
    1504,  1506,  1508,  1510,  1512,  1514,  1516,  1521,  1523,  1528,
    1531,  1533,  1536,  1539,  1542,  1545,  1548,  1551,  1554,  1557,
    1560,  1562,  1565,  1567,  1570,  1573,  1576,  1579,  1582,  1584,
    1586,  1588,  1590,  1592,  1594,  1596,  1598,  1600,  1605,  1607,
    1609,  1611,  1613,  1615,  1617,  1619,  1621,  1623,  1625,  1627,
    1629,  1631,  1633,  1635,  1638,  1640,  1644,  1647,  1649,  1654,
    1659,  1663,  1668,  1672,  1674,  1676,  1678,  1680,  1682,  1684,
    1686,  1688,  1694,  1705,  1707,  1709,  1710,  1718,  1726,  1727,
    1729,  1733,  1737,  1741,  1745,  1746,  1749,  1752,  1753,  1756,
    1759,  1760,  1763,  1766,  1767,  1770,  1773,  1774,  1777,  1780,
    1783,  1786,  1790,  1791,  1793,  1797,  1799,  1802,  1804,  1806,
    1808,  1810,  1812,  1814,  1816,  1821,  1825,  1827,  1831,  1834,
    1838,  1842,  1844,  1848,  1852,  1857,  1862,  1866,  1868,  1872,
    1875,  1879,  1882,  1883,  1885,  1886,  1889,  1892,  1896,  1897,
    1899,  1901,  1903,  1905,  1909,  1911,  1913,  1915,  1917,  1919,
    1921,  1923,  1925,  1927,  1929,  1931,  1933,  1935,  1937,  1941,
    1945,  1950,  1955,  1959,  1963,  1967,  1971,  1976,  1981,  1985,
    1990,  1997,  1999,  2001,  2006,  2008,  2012,  2017,  2024,  2032,
    2034,  2036,  2041,  2043,  2045,  2047,  2049,  2051,  2055,  2056,
    2059,  2060,  2063,  2067,  2069,  2073,  2075,  2080,  2082,  2085,
    2087,  2089,  2092,  2095,  2099,  2101,  2104,  2109,  2113,  2118,
    2119,  2121,  2124,  2126,  2129,  2131,  2132,  2136,  2139,  2143,
    2147,  2152,  2154,  2158,  2162,  2165,  2168,  2171,  2174,  2177,
    2179,  2181,  2183,  2185,  2187,  2190,  2194,  2196,  2198,  2200,
    2202,  2204,  2206,  2208,  2210,  2213,  2216,  2218,  2220,  2223,
    2226,  2228,  2231,  2234,  2237,  2243,  2249,  2252,  2253,  2257,
    2260,  2265,  2270,  2277,  2282,  2286,  2291,  2292,  2295,  2298,
    2302,  2304,  2308,  2311,  2314,  2317,  2319,  2322,  2326,  2330,
    2334,  2338,  2342,  2349,  2356,  2361,  2366,  2371,  2374,  2377,
    2380,  2383,  2386,  2394,  2400,  2406,  2413,  2417,  2419,  2424,
    2426,  2428,  2430,  2432,  2434,  2436,  2442,  2450,  2459,  2470,
    2477,  2486,  2487,  2490,  2493,  2497,  2498,  2501,  2503,  2507,
    2509,  2511,  2513,  2517,  2525,  2529,  2537,  2543,  2553,  2555,
    2557,  2559,  2561,  2565,  2571,  2573,  2575,  2579,  2584,  2588,
    2593,  2595,  2597,  2602,  2608,  2615,  2617,  2621,  2625,  2627,
    2629,  2631,  2633,  2635,  2637,  2639,  2641,  2643,  2645,  2647,
    2649,  2651,  2653,  2655,  2657,  2659,  2661,  2663,  2665,  2667,
    2669,  2671,  2673,  2675,  2677,  2679,  2681,  2683,  2685,  2687,
    2689,  2691,  2693,  2695,  2697,  2699,  2701,  2703,  2705,  2707,
    2709,  2711,  2713,  2715,  2717,  2719,  2721,  2723,  2725,  2727,
    2729,  2731,  2733,  2735,  2737,  2739,  2741,  2743,  2745,  2747,
    2749,  2751,  2753,  2755,  2757,  2759,  2761,  2763,  2765,  2767,
    2769,  2771,  2773,  2775,  2777,  2779,  2781,  2783,  2785,  2787,
    2789,  2791,  2793,  2795,  2797,  2799,  2801,  2803,  2805,  2807,
    2809,  2811,  2813,  2815,  2817,  2819,  2821,  2823,  2825,  2827,
    2829,  2831,  2833,  2835,  2837,  2839,  2841,  2843,  2845,  2847,
    2849,  2851,  2853,  2855,  2857,  2859,  2861,  2863,  2865,  2867,
    2869,  2871,  2873,  2875,  2877,  2879,  2881,  2883,  2885,  2887,
    2889,  2891,  2893,  2895,  2897,  2899,  2901,  2903,  2905,  2907,
    2909,  2911,  2913,  2915,  2917,  2919,  2921,  2923,  2925,  2927,
    2929,  2931,  2933,  2935,  2937,  2939,  2941,  2943,  2945,  2947,
    2949,  2951,  2953,  2955,  2957,  2959,  2961,  2963,  2965,  2967,
    2969,  2971
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     325,     0,    -1,     3,   326,   460,    -1,     4,   327,   419,
      -1,     6,   327,   328,    -1,     5,   335,    -1,     7,   331,
      -1,     8,   330,    -1,    -1,    -1,   180,   453,   450,   670,
     451,   456,   459,   329,    -1,    -1,   429,    -1,   453,   451,
     456,    -1,   332,   333,    -1,   190,   173,   647,    37,   598,
     429,    -1,    -1,   333,   334,    -1,   180,   453,   450,   670,
     451,   456,   459,   429,    -1,   336,    -1,   249,   337,   338,
     248,    -1,   358,    -1,    -1,   337,   278,    -1,   337,   287,
      -1,    -1,   338,   339,    -1,   338,   343,    -1,   338,   348,
      -1,   338,   350,    -1,   338,   352,    -1,   338,   356,    -1,
     340,   345,   361,   248,    -1,   250,    -1,   340,   301,   409,
      -1,   340,   280,    -1,   340,   304,   342,    -1,   340,   279,
     341,    -1,   340,   302,   576,    -1,    -1,   341,   122,    -1,
     341,   318,    -1,   341,   319,    -1,    52,    -1,    53,    -1,
      54,    -1,   344,   345,   361,   248,    -1,   258,    -1,   344,
     280,    -1,   344,   302,   576,    -1,   344,   292,    -1,    -1,
     345,   346,    -1,   347,   361,   248,    -1,   257,    -1,   347,
     280,    -1,   347,   303,   461,    -1,   347,   302,   576,    -1,
     347,   291,    -1,   347,   290,    -1,   349,   361,   248,    -1,
     257,    -1,   349,   280,    -1,   349,   303,   461,    -1,   349,
     302,   576,    -1,   349,   291,    -1,   349,   290,    -1,   351,
     361,   248,    -1,   263,    -1,   351,   280,    -1,   351,   303,
     461,    -1,   351,   302,   576,    -1,   353,   248,    -1,   276,
      -1,   353,   280,    -1,   353,   310,   354,    -1,   353,   295,
      -1,   353,   311,   355,    -1,   353,   281,    -1,   353,   282,
      -1,   353,   283,    -1,   353,   296,    -1,   353,   297,    -1,
     353,   298,    -1,   353,   284,    -1,   353,   285,    -1,   353,
     299,    -1,   353,   286,    -1,   353,   300,    -1,   353,   312,
     355,    -1,   353,   278,    -1,   321,    -1,   322,    -1,   323,
      -1,    63,    -1,   122,    -1,   122,    -1,   355,   122,    -1,
     357,   248,    -1,   277,    -1,   277,   288,    -1,   277,   289,
      -1,   277,   288,   289,    -1,   277,   289,   288,    -1,   313,
     359,   361,   248,    -1,    -1,   359,   315,   360,    -1,   359,
     314,    -1,    -1,   360,   459,    -1,   360,   115,    -1,    -1,
     361,   366,    -1,   361,   358,    -1,   361,   362,    -1,   363,
      -1,   365,    -1,   367,    -1,   371,    -1,   376,    -1,   380,
      -1,   378,    -1,   389,    -1,   391,    -1,   393,    -1,   395,
      -1,   397,    -1,   399,    -1,   401,    -1,   385,    -1,   403,
      -1,   405,    -1,   407,    -1,   364,   361,   248,    -1,   251,
      -1,   364,   303,   461,    -1,   364,   306,   360,    -1,   252,
     366,   248,    -1,   316,    -1,   317,    -1,   368,   370,   248,
      -1,   253,    -1,   368,   303,   461,    -1,   368,   279,   369,
      -1,   122,    -1,   318,    -1,   320,    -1,    -1,   370,   374,
      -1,   372,   373,   248,    -1,   254,    -1,   372,   280,    -1,
      -1,   373,   374,    -1,   375,   361,   248,    -1,   255,    -1,
     375,   280,    -1,   375,   303,   461,    -1,   375,   302,   576,
      -1,   375,   290,    -1,   377,   248,    -1,   256,   303,   461,
      -1,   379,   361,   248,    -1,   262,   305,   461,    -1,   259,
     381,   384,   248,    -1,   382,    -1,   381,   382,    -1,   383,
     361,   248,    -1,   260,   305,   461,    -1,    -1,   261,   361,
     248,    -1,   386,   387,   388,   248,    -1,   270,    -1,   386,
     303,   461,    -1,   386,   308,   360,    -1,   386,   309,   360,
      -1,    -1,   271,   361,   248,    -1,    -1,   272,   361,   248,
      -1,   390,   361,   248,   361,    -1,   263,    -1,   390,   280,
      -1,   390,   303,   461,    -1,   390,   302,   576,    -1,   392,
     361,   248,    -1,   264,    -1,   392,   303,   461,    -1,   394,
     361,   248,    -1,   265,    -1,   394,   280,   360,    -1,   394,
     303,   461,    -1,   396,   361,   248,    -1,   266,    -1,   398,
     361,   248,    -1,   267,    -1,   398,   280,   360,    -1,   398,
     307,   360,    -1,   398,   303,   461,    -1,   398,   306,   360,
      -1,   400,   361,   248,    -1,   268,    -1,   400,   280,   360,
      -1,   400,   303,   461,    -1,   402,   361,   248,    -1,   269,
      -1,   402,   280,   360,    -1,   402,   307,   360,    -1,   404,
     248,    -1,   273,    -1,   404,   303,   461,    -1,   404,   293,
      -1,   406,   361,   248,    -1,   274,    -1,   406,   293,    -1,
     406,   294,    -1,   408,   361,   248,    -1,   275,    -1,   408,
     303,   461,    -1,   409,    12,   410,    -1,   410,    -1,   415,
      -1,    36,    -1,   412,    -1,    36,    -1,    30,    -1,   232,
      40,   413,    41,    -1,   233,    40,   646,    33,   414,    41,
      -1,   646,    -1,   534,    -1,   532,    -1,   534,    -1,   416,
      -1,   411,   416,    -1,   412,    36,   416,    -1,   412,    30,
     416,    -1,   415,    36,   416,    -1,   415,    30,   416,    -1,
     417,   418,    -1,   527,    -1,    71,    19,   527,    -1,    55,
      19,   527,    -1,    20,   527,    -1,    -1,   418,    21,   461,
      22,    -1,   420,   423,    -1,   420,   424,    -1,   423,    -1,
     424,    -1,   186,   421,   429,    -1,   186,   421,   422,   429,
      -1,   186,   422,   429,    -1,   187,   125,    -1,   176,   125,
      -1,   426,   460,    -1,   425,   426,    -1,   190,   173,   647,
      37,   598,   429,    -1,    -1,   426,   427,   429,    -1,   426,
     428,   429,    -1,   426,   430,   429,    -1,   426,   432,   429,
      -1,   426,   446,   429,    -1,   426,   449,   429,    -1,   426,
     649,   429,    -1,   426,   433,   429,    -1,   426,   434,   429,
      -1,   431,    -1,   440,    -1,   441,    -1,   448,    -1,   435,
      -1,   436,    -1,   633,    -1,   437,    -1,   442,    -1,   445,
      -1,    50,    -1,   180,   173,   647,    37,   598,    -1,   180,
      65,   171,    -1,   180,    65,   172,    -1,   180,   183,   191,
     173,   598,    -1,   180,   183,   450,   173,   598,    -1,   180,
     185,   669,   125,    -1,   180,    66,   616,    -1,   180,   182,
     120,    -1,   180,   182,   121,    -1,   180,   183,   138,   142,
     143,    -1,   180,   183,   138,   142,   144,    -1,   180,   184,
     438,    33,   439,    -1,   171,    -1,   177,    -1,   178,    -1,
     179,    -1,   180,   183,   145,   598,    -1,   180,    67,   598,
      -1,   188,   189,   444,   598,   443,    -1,   188,   189,   598,
     443,    -1,    -1,   114,   598,    -1,   443,    33,   598,    -1,
     173,   647,    37,    -1,   183,   191,   173,    -1,   188,   190,
     173,   647,    37,   598,   443,    -1,   188,   190,   598,   443,
      -1,   180,   126,     9,   535,   575,   447,    -1,    10,   462,
      -1,   175,    -1,   180,   181,   171,    -1,   180,   181,   172,
      -1,   180,   453,   450,   670,   451,   456,   452,    -1,   192,
      -1,   193,    -1,    40,    41,    -1,    40,   457,    41,    -1,
     459,    -1,   175,    -1,    -1,   453,   454,    -1,   453,   455,
      -1,   453,   231,    -1,   244,    -1,   245,    -1,   246,    -1,
     247,    -1,    -1,   152,   576,    -1,   457,    33,   458,    -1,
     458,    -1,     9,   535,   575,    -1,    48,   461,    49,    -1,
     461,    -1,   461,    33,   462,    -1,   462,    -1,   463,    -1,
     483,    -1,   486,    -1,   492,    -1,   634,    -1,   636,    -1,
     639,    -1,   638,    -1,   640,    -1,   493,    -1,   655,    -1,
     656,    -1,   464,   131,   462,    -1,   465,    -1,   464,   477,
      -1,   464,   466,    -1,   467,    -1,   472,    -1,   465,    -1,
     475,    -1,   476,    -1,   132,   468,    -1,   468,    33,   469,
      -1,   469,    -1,     9,   535,   575,   470,   471,   133,   462,
      -1,    -1,   114,     9,   535,    -1,    -1,   194,     9,   535,
      -1,   134,   473,    -1,   473,    33,   474,    -1,   474,    -1,
       9,   535,   575,    10,   462,    -1,   194,     9,   535,    10,
     462,    -1,   135,   462,    -1,   136,     9,   535,    -1,   138,
     137,   478,    -1,   139,   138,   137,   478,    -1,   478,    33,
     479,    -1,   479,    -1,   480,   481,   482,    -1,   480,   481,
     482,   145,   598,    -1,   462,    -1,    -1,   140,    -1,   141,
      -1,    -1,   142,   143,    -1,   142,   144,    -1,   146,   484,
     148,   462,    -1,   147,   484,   148,   462,    -1,   484,    33,
     485,    -1,   485,    -1,     9,   535,   575,   133,   462,    -1,
     149,    40,   461,    41,   487,   488,    -1,   487,   489,    -1,
     489,    -1,   183,     9,   535,   131,   462,    -1,   183,   131,
     462,    -1,   490,   131,   462,    -1,   150,   491,   576,    -1,
     490,    12,   576,    -1,    -1,     9,   535,   152,    -1,   153,
      40,   461,    41,   154,   462,   155,   462,    -1,   493,   156,
     494,    -1,   494,    -1,   494,   157,   495,    -1,   495,    -1,
     497,    37,   497,    -1,   497,    13,   497,    -1,    -1,   497,
      24,   496,   497,    -1,   497,    14,   497,    -1,   497,    15,
     497,    -1,   497,    16,   497,    -1,   497,   108,   497,    -1,
     497,   109,   497,    -1,   497,   110,   497,    -1,   497,   111,
     497,    -1,   497,   112,   497,    -1,   497,   113,   497,    -1,
     497,   170,   497,    -1,   497,    17,   497,    -1,   497,    18,
     497,    -1,   497,    -1,   498,   195,    63,   599,    -1,   498,
     195,    63,   599,   632,    -1,   498,    -1,   499,   161,   499,
      -1,   499,    -1,   499,    34,   500,    -1,   499,    35,   500,
      -1,   500,    -1,   500,    11,   501,    -1,   500,   162,   501,
      -1,   500,    70,   501,    -1,   500,   163,   501,    -1,   501,
      -1,   501,    12,   502,    -1,   501,   164,   502,    -1,   502,
      -1,   502,   165,   503,    -1,   502,   166,   503,    -1,   503,
      -1,   504,   158,   159,   576,    -1,   504,    -1,   505,   169,
     152,   576,    -1,   505,    -1,   506,   160,   152,   573,    -1,
     506,    -1,   507,   168,   152,   573,    -1,   507,    -1,    35,
     507,    -1,    34,   507,    -1,   508,    -1,   509,    -1,   514,
      -1,   510,    -1,   167,    48,   461,    49,    -1,   167,    68,
      48,   461,    49,    -1,   167,    69,    48,   461,    49,    -1,
     511,    48,    49,    -1,   511,    48,   461,    49,    -1,   512,
      -1,   511,   512,    -1,    32,   548,   669,   513,    -1,   130,
      -1,   105,   130,    -1,   515,    -1,   515,   516,    -1,    30,
     516,    -1,   516,    -1,    36,    -1,   516,    36,   517,    -1,
     516,    30,   517,    -1,   517,    -1,   518,    -1,   530,    -1,
     519,    -1,   520,    -1,   521,    -1,   519,    21,   461,    22,
      -1,   524,    -1,   520,    21,   461,    22,    -1,   522,   527,
      -1,   523,    -1,    71,    19,    -1,    72,    19,    -1,    55,
      19,    -1,    73,    19,    -1,    74,    19,    -1,    75,    19,
      -1,    76,    19,    -1,   173,    19,    -1,    20,   527,    -1,
     527,    -1,   525,   527,    -1,   526,    -1,    77,    19,    -1,
      78,    19,    -1,    79,    19,    -1,    80,    19,    -1,    81,
      19,    -1,    39,    -1,   580,    -1,   528,    -1,   648,    -1,
     529,    -1,    11,    -1,   127,    -1,   128,    -1,   531,    -1,
     530,    21,   461,    22,    -1,   663,    -1,   532,    -1,   534,
      -1,   536,    -1,   537,    -1,   540,    -1,   543,    -1,   538,
      -1,   539,    -1,   660,    -1,   533,    -1,   646,    -1,   643,
      -1,   644,    -1,   645,    -1,     9,   535,    -1,   669,    -1,
      40,   461,    41,    -1,    40,    41,    -1,    38,    -1,   120,
      48,   461,    49,    -1,   121,    48,   461,    49,    -1,   670,
      40,    41,    -1,   670,    40,   541,    41,    -1,   541,    33,
     542,    -1,   542,    -1,   462,    -1,    23,    -1,   544,    -1,
     560,    -1,   545,    -1,   556,    -1,   558,    -1,    24,   546,
     547,   548,    42,    -1,    24,   546,   547,   548,    25,   555,
      31,   546,   548,    26,    -1,   122,    -1,   123,    -1,    -1,
     547,   105,   122,   548,    43,   548,   549,    -1,   547,   105,
     123,   548,    43,   548,   550,    -1,    -1,   105,    -1,    44,
     551,    45,    -1,    46,   553,    47,    -1,    44,   552,    45,
      -1,    46,   554,    47,    -1,    -1,   551,   459,    -1,   551,
     115,    -1,    -1,   552,   459,    -1,   552,   115,    -1,    -1,
     553,   459,    -1,   553,   116,    -1,    -1,   554,   459,    -1,
     554,   116,    -1,    -1,   555,   544,    -1,   555,   459,    -1,
     555,   118,    -1,   555,   117,    -1,    28,   557,    29,    -1,
      -1,   107,    -1,    27,   129,   559,    -1,   106,    -1,   105,
     106,    -1,   561,    -1,   562,    -1,   565,    -1,   567,    -1,
     568,    -1,   569,    -1,   570,    -1,    82,    48,   461,    49,
      -1,   191,   563,   564,    -1,   124,    -1,    48,   461,    49,
      -1,    48,    49,    -1,    48,   461,    49,    -1,    55,   566,
     564,    -1,   124,    -1,    48,   461,    49,    -1,   173,   571,
     564,    -1,    63,    48,   461,    49,    -1,    56,    48,   461,
      49,    -1,    60,   571,   572,    -1,   124,    -1,    48,   461,
      49,    -1,    48,    49,    -1,    48,   461,    49,    -1,   579,
     574,    -1,    -1,    23,    -1,    -1,   152,   576,    -1,   578,
     577,    -1,    64,    40,    41,    -1,    -1,    11,    -1,    34,
      -1,    23,    -1,   579,    -1,   174,    40,    41,    -1,   580,
      -1,   664,    -1,   668,    -1,   648,    -1,   582,    -1,   591,
      -1,   587,    -1,   593,    -1,   589,    -1,   586,    -1,   584,
      -1,   583,    -1,   585,    -1,   581,    -1,    59,    40,    41,
      -1,    57,    40,    41,    -1,    57,    40,   591,    41,    -1,
      57,    40,   593,    41,    -1,    63,    40,    41,    -1,    56,
      40,    41,    -1,    58,    40,    41,    -1,    60,    40,    41,
      -1,    60,    40,   647,    41,    -1,    60,    40,   125,    41,
      -1,    55,    40,    41,    -1,    55,    40,   588,    41,    -1,
      55,    40,   588,    33,   597,    41,    -1,   595,    -1,    11,
      -1,    61,    40,   590,    41,    -1,   595,    -1,   191,    40,
      41,    -1,   191,    40,   592,    41,    -1,   191,    40,   592,
      33,   597,    41,    -1,   191,    40,   592,    33,   597,    23,
      41,    -1,   596,    -1,    11,    -1,    62,    40,   594,    41,
      -1,   596,    -1,   648,    -1,   648,    -1,   648,    -1,   125,
      -1,   602,   601,   600,    -1,    -1,   196,   498,    -1,    -1,
     601,   613,    -1,   602,   241,   603,    -1,   603,    -1,   603,
     242,   604,    -1,   604,    -1,   604,    83,   133,   605,    -1,
     605,    -1,   243,   606,    -1,   606,    -1,   607,    -1,   607,
     616,    -1,   608,   611,    -1,    40,   599,    41,    -1,   609,
      -1,   532,   610,    -1,    48,   461,    49,   610,    -1,   511,
      48,    49,    -1,   511,    48,   599,    49,    -1,    -1,   208,
      -1,   208,    97,    -1,   209,    -1,   209,   213,    -1,   210,
      -1,    -1,   199,   612,   200,    -1,   211,   499,    -1,   119,
     144,   499,    -1,   119,   101,   499,    -1,   212,   499,   161,
     499,    -1,   120,    -1,   197,   499,   614,    -1,   198,   612,
     614,    -1,   201,   615,    -1,   202,   615,    -1,   114,    99,
      -1,   114,   100,    -1,    95,    96,    -1,   213,    -1,   214,
      -1,   215,    -1,   216,    -1,   217,    -1,    84,   617,    -1,
     616,    84,   617,    -1,   629,    -1,   630,    -1,   621,    -1,
     620,    -1,   618,    -1,   619,    -1,   624,    -1,   631,    -1,
     151,    87,    -1,   151,    86,    -1,   203,    -1,   204,    -1,
      88,    87,    -1,    88,    86,    -1,    91,    -1,    85,    91,
      -1,    92,   623,    -1,    92,   183,    -1,    92,    40,   623,
     622,    41,    -1,    92,    40,   183,   622,    41,    -1,    85,
      92,    -1,    -1,   622,    33,   623,    -1,   114,   598,    -1,
     114,   598,   205,   646,    -1,   114,   598,   612,   206,    -1,
     114,   598,   205,   646,   612,   206,    -1,    93,   213,   626,
     625,    -1,    85,    93,   213,    -1,    93,   213,   183,   625,
      -1,    -1,   625,   628,    -1,   114,   598,    -1,    40,   627,
      41,    -1,   646,    -1,   627,    33,   646,    -1,   164,   626,
      -1,   166,   626,    -1,   207,   646,    -1,    94,    -1,    85,
      94,    -1,   185,   669,   125,    -1,    90,    96,   501,    -1,
     180,   224,    69,    -1,   180,   224,    68,    -1,   180,   224,
     102,    -1,   635,   462,   152,   220,   230,   462,    -1,   635,
     462,   152,   228,   230,   462,    -1,   635,   462,   230,   462,
      -1,   635,   462,   223,   462,    -1,   635,   462,   222,   462,
      -1,   221,    59,    -1,   221,   226,    -1,   637,   462,    -1,
     229,    59,    -1,   229,   226,    -1,   218,   104,   159,    59,
     462,   225,   462,    -1,   218,    59,   462,   225,   462,    -1,
     227,    59,   462,   152,   462,    -1,   103,   641,   219,   462,
     131,   462,    -1,   641,    33,   642,    -1,   642,    -1,     9,
     535,    10,   462,    -1,    52,    -1,    53,    -1,    54,    -1,
     125,    -1,   669,    -1,   669,    -1,   180,   234,   236,   669,
     459,    -1,   180,   234,   236,   669,   651,   650,   459,    -1,
     180,   234,   236,   669,   652,   235,   409,   459,    -1,   180,
     234,   236,   669,   652,   235,   409,   651,   650,   459,    -1,
     180,   234,   652,   235,   409,   459,    -1,   180,   234,   652,
     235,   409,   651,   650,   459,    -1,    -1,   152,   576,    -1,
      40,    41,    -1,    40,   457,    41,    -1,    -1,   240,   653,
      -1,   654,    -1,   653,    33,   654,    -1,   122,    -1,   318,
      -1,   319,    -1,   237,   234,   669,    -1,   237,   234,   669,
     225,    40,   658,    41,    -1,   238,   239,   462,    -1,   238,
     239,   462,   225,    40,   658,    41,    -1,   238,   239,   462,
     240,   657,    -1,   238,   239,   462,   240,   657,   225,    40,
     658,    41,    -1,   669,    -1,   318,    -1,   320,    -1,   659,
      -1,   658,    33,   659,    -1,     9,   535,   575,    10,   462,
      -1,   661,    -1,   662,    -1,   670,    51,    52,    -1,   193,
     451,   575,   459,    -1,   530,    40,    41,    -1,   530,    40,
     541,    41,    -1,   665,    -1,   666,    -1,   193,    40,    11,
      41,    -1,   193,    40,    41,   152,   576,    -1,   193,    40,
     667,    41,   152,   576,    -1,   576,    -1,   667,    33,   576,
      -1,    40,   578,    41,    -1,   670,    -1,    55,    -1,    56,
      -1,    57,    -1,   191,    -1,   174,    -1,   153,    -1,    59,
      -1,    60,    -1,    61,    -1,    62,    -1,    63,    -1,   149,
      -1,    64,    -1,   193,    -1,    58,    -1,   122,    -1,    65,
      -1,    66,    -1,    67,    -1,    68,    -1,    69,    -1,    70,
      -1,    71,    -1,    72,    -1,    73,    -1,    74,    -1,    75,
      -1,    76,    -1,    77,    -1,    78,    -1,    79,    -1,    80,
      -1,    81,    -1,    82,    -1,    83,    -1,    86,    -1,    87,
      -1,    88,    -1,    89,    -1,    91,    -1,    92,    -1,    93,
      -1,    94,    -1,    95,    -1,    96,    -1,    97,    -1,    98,
      -1,    99,    -1,   100,    -1,   101,    -1,   102,    -1,   103,
      -1,   104,    -1,   108,    -1,   109,    -1,   110,    -1,   111,
      -1,   112,    -1,   113,    -1,   114,    -1,   126,    -1,   131,
      -1,   132,    -1,   133,    -1,   134,    -1,   135,    -1,   137,
      -1,   138,    -1,   139,    -1,   140,    -1,   141,    -1,   142,
      -1,   143,    -1,   144,    -1,   145,    -1,   146,    -1,   147,
      -1,   148,    -1,   150,    -1,   152,    -1,   154,    -1,   155,
      -1,   156,    -1,   157,    -1,   158,    -1,   159,    -1,   160,
      -1,   161,    -1,   162,    -1,   163,    -1,   164,    -1,   165,
      -1,   166,    -1,   167,    -1,   168,    -1,   169,    -1,   170,
      -1,   171,    -1,   172,    -1,   173,    -1,   175,    -1,   176,
      -1,   177,    -1,   178,    -1,   179,    -1,   180,    -1,   181,
      -1,   182,    -1,   183,    -1,   184,    -1,   185,    -1,   187,
      -1,   188,    -1,   189,    -1,   192,    -1,   194,    -1,    84,
      -1,    85,    -1,   195,    -1,   196,    -1,   197,    -1,   198,
      -1,   199,    -1,   200,    -1,   201,    -1,   202,    -1,   203,
      -1,   204,    -1,   205,    -1,   206,    -1,   207,    -1,   208,
      -1,   209,    -1,   210,    -1,   211,    -1,   212,    -1,   213,
      -1,   214,    -1,   215,    -1,   216,    -1,   217,    -1,   218,
      -1,   219,    -1,   220,    -1,   221,    -1,   222,    -1,   223,
      -1,   224,    -1,   225,    -1,   226,    -1,   227,    -1,   228,
      -1,   229,    -1,   230,    -1,   231,    -1,   120,    -1,   121,
      -1,   232,    -1,   233,    -1,   234,    -1,   235,    -1,   236,
      -1,   237,    -1,   238,    -1,   239,    -1,   240,    -1,   241,
      -1,   242,    -1,   243,    -1,   244,    -1,   245,    -1,   246,
      -1,   247,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   765,   765,   766,   767,   768,   769,   770,   775,   782,
     793,   801,   801,   804,   813,   824,   862,   865,   871,   907,
     938,   941,   963,   965,   969,   976,   978,   979,   980,   981,
     982,   983,   987,  1006,  1011,  1016,  1023,  1028,  1033,  1042,
    1045,  1051,  1056,  1063,  1063,  1063,  1066,  1079,  1084,  1091,
    1096,  1105,  1108,  1116,  1143,  1147,  1154,  1160,  1165,  1170,
    1179,  1203,  1207,  1214,  1219,  1224,  1229,  1240,  1258,  1262,
    1269,  1274,  1282,  1307,  1310,  1313,  1316,  1319,  1322,  1325,
    1328,  1331,  1334,  1337,  1340,  1343,  1346,  1349,  1352,  1355,
    1358,  1364,  1365,  1366,  1367,  1368,  1380,  1383,  1388,  1391,
    1396,  1401,  1409,  1417,  1428,  1439,  1442,  1449,  1459,  1462,
    1467,  1478,  1481,  1486,  1491,  1499,  1500,  1501,  1502,  1503,
    1504,  1505,  1506,  1507,  1508,  1509,  1510,  1511,  1512,  1513,
    1514,  1515,  1516,  1520,  1538,  1542,  1547,  1556,  1562,  1562,
    1565,  1585,  1589,  1594,  1602,  1607,  1611,  1619,  1622,  1630,
    1643,  1647,  1658,  1661,  1669,  1687,  1691,  1698,  1703,  1708,
    1716,  1722,  1730,  1738,  1745,  1759,  1763,  1777,  1786,  1794,
    1797,  1804,  1839,  1843,  1848,  1854,  1864,  1867,  1875,  1878,
    1885,  1903,  1907,  1914,  1919,  1927,  1945,  1949,  1957,  1979,
    1983,  1990,  1998,  2007,  2015,  2052,  2056,  2063,  2070,  2079,
    2088,  2115,  2119,  2126,  2138,  2164,  2168,  2175,  2185,  2197,
    2201,  2206,  2215,  2228,  2232,  2237,  2246,  2260,  2264,  2277,
    2282,  2294,  2298,  2306,  2319,  2327,  2340,  2347,  2361,  2361,
    2364,  2364,  2386,  2390,  2394,  2412,  2430,  2440,  2454,  2462,
    2468,  2474,  2480,  2492,  2495,  2509,  2510,  2511,  2512,  2518,
    2519,  2520,  2524,  2544,  2561,  2564,  2568,  2580,  2582,  2586,
    2590,  2594,  2598,  2602,  2608,  2614,  2618,  2632,  2633,  2634,
    2635,  2636,  2637,  2638,  2639,  2643,  2643,  2646,  2650,  2658,
    2663,  2672,  2677,  2686,  2746,  2754,  2759,  2768,  2773,  2782,
    2792,  2796,  2804,  2808,  2816,  2832,  2841,  2853,  2862,  2865,
    2870,  2879,  2883,  2891,  2901,  2913,  2920,  2924,  2932,  2937,
    2946,  2954,  2954,  2957,  2961,  2969,  2970,  2979,  2982,  2995,
    3008,  3018,  3018,  3021,  3021,  3025,  3028,  3036,  3041,  3051,
    3059,  3067,  3080,  3095,  3102,  3103,  3104,  3105,  3106,  3107,
    3108,  3109,  3110,  3111,  3112,  3113,  3118,  3162,  3163,  3176,
    3183,  3183,  3186,  3186,  3186,  3191,  3198,  3202,  3206,  3218,
    3221,  3232,  3235,  3245,  3252,  3256,  3260,  3264,  3277,  3285,
    3293,  3298,  3306,  3310,  3316,  3323,  3338,  3355,  3358,  3362,
    3370,  3378,  3382,  3391,  3399,  3410,  3414,  3418,  3429,  3436,
    3445,  3449,  3453,  3461,  3479,  3484,  3494,  3497,  3505,  3513,
    3522,  3527,  3536,  3546,  3550,  3554,  3554,  3558,  3562,  3566,
    3570,  3574,  3578,  3582,  3586,  3590,  3594,  3598,  3602,  3606,
    3611,  3615,  3619,  3624,  3631,  3636,  3640,  3644,  3649,  3653,
    3657,  3661,  3665,  3670,  3674,  3678,  3683,  3687,  3691,  3696,
    3713,  3718,  3723,  3728,  3732,  3737,  3741,  3746,  3752,  3758,
    3763,  3764,  3765,  3773,  3777,  3781,  3789,  3796,  3805,  3805,
    3809,  3829,  3830,  3838,  3839,  3845,  3866,  3870,  3884,  3890,
    3903,  3907,  3907,  3911,  3912,  3921,  3922,  3928,  3929,  3938,
    3950,  3961,  3965,  3969,  3973,  3977,  3981,  3985,  3989,  3997,
    4005,  4022,  4030,  4039,  4043,  4047,  4051,  4055,  4063,  4075,
    4080,  4085,  4092,  4097,  4104,  4111,  4123,  4124,  4128,  4134,
    4135,  4136,  4137,  4138,  4139,  4140,  4141,  4142,  4146,  4146,
    4149,  4149,  4149,  4153,  4160,  4164,  4168,  4176,  4184,  4194,
    4204,  4208,  4231,  4236,  4246,  4247,  4256,  4260,  4269,  4269,
    4269,  4273,  4279,  4288,  4288,  4293,  4296,  4305,  4316,  4316,
    4321,  4325,  4332,  4336,  4347,  4350,  4355,  4366,  4369,  4373,
    4387,  4390,  4395,  4406,  4409,  4413,  4431,  4434,  4439,  4444,
    4451,  4464,  4478,  4481,  4486,  4505,  4506,  4520,  4521,  4522,
    4523,  4524,  4525,  4526,  4531,  4539,  4547,  4551,  4559,  4563,
    4572,  4579,  4583,  4593,  4601,  4609,  4617,  4624,  4634,  4641,
    4645,  4653,  4664,  4667,  4676,  4679,  4689,  4696,  4706,  4707,
    4709,  4711,  4717,  4721,  4725,  4726,  4727,  4731,  4743,  4744,
    4745,  4746,  4747,  4748,  4749,  4750,  4751,  4752,  4757,  4765,
    4769,  4774,  4783,  4791,  4799,  4808,  4812,  4816,  4824,  4828,
    4832,  4840,  4841,  4849,  4857,  4862,  4866,  4870,  4874,  4883,
    4884,  4892,  4899,  4902,  4905,  4908,  4912,  4924,  4940,  4942,
    4949,  4952,  4961,  4973,  4978,  4990,  4995,  4999,  5004,  5009,
    5014,  5015,  5024,  5029,  5033,  5039,  5043,  5051,  5056,  5066,
    5069,  5073,  5077,  5081,  5085,  5093,  5095,  5106,  5112,  5118,
    5124,  5139,  5143,  5147,  5151,  5155,  5159,  5163,  5167,  5175,
    5179,  5183,  5191,  5195,  5203,  5204,  5216,  5217,  5218,  5219,
    5220,  5221,  5222,  5223,  5231,  5235,  5239,  5243,  5252,  5256,
    5264,  5268,  5278,  5282,  5286,  5290,  5294,  5302,  5304,  5311,
    5315,  5319,  5323,  5333,  5337,  5341,  5349,  5351,  5359,  5363,
    5370,  5374,  5382,  5386,  5394,  5402,  5406,  5414,  5421,  5429,
    5434,  5439,  5451,  5455,  5459,  5463,  5467,  5473,  5473,  5478,
    5484,  5484,  5489,  5493,  5503,  5511,  5518,  5523,  5531,  5540,
    5552,  5564,  5577,  5588,  5597,  5609,  5614,  5619,  5625,  5631,
    5637,  5647,  5650,  5658,  5662,  5670,  5674,  5682,  5687,  5696,
    5700,  5704,  5712,  5716,  5724,  5728,  5732,  5736,  5744,  5748,
    5752,  5760,  5765,  5775,  5788,  5788,  5792,  5800,  5811,  5816,
    5840,  5840,  5844,  5852,  5856,  5863,  5868,  5877,  5886,  5886,
    5886,  5886,  5886,  5886,  5886,  5886,  5887,  5887,  5887,  5887,
    5887,  5887,  5888,  5888,  5892,  5892,  5892,  5892,  5892,  5892,
    5892,  5892,  5892,  5892,  5893,  5893,  5893,  5893,  5893,  5893,
    5893,  5894,  5894,  5894,  5894,  5894,  5894,  5894,  5894,  5895,
    5895,  5895,  5895,  5895,  5895,  5895,  5895,  5895,  5895,  5895,
    5896,  5896,  5896,  5896,  5896,  5896,  5896,  5896,  5896,  5896,
    5896,  5896,  5897,  5897,  5897,  5897,  5897,  5897,  5897,  5897,
    5897,  5897,  5898,  5898,  5898,  5898,  5898,  5898,  5898,  5898,
    5898,  5898,  5898,  5898,  5899,  5899,  5899,  5899,  5899,  5899,
    5899,  5899,  5899,  5899,  5899,  5899,  5900,  5900,  5900,  5900,
    5900,  5900,  5900,  5900,  5900,  5901,  5901,  5901,  5901,  5901,
    5901,  5901,  5901,  5902,  5902,  5902,  5902,  5902,  5902,  5902,
    5902,  5902,  5902,  5902,  5903,  5903,  5903,  5903,  5903,  5903,
    5903,  5903,  5903,  5904,  5904,  5904,  5904,  5904,  5904,  5904,
    5904,  5904,  5904,  5905,  5905,  5905,  5905,  5905,  5905,  5905,
    5905,  5905,  5905,  5905,  5906,  5906,  5906,  5906,  5906,  5906,
    5906,  5906,  5906,  5906,  5906,  5907,  5907,  5907,  5907,  5907,
    5907,  5907
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"<end of file>\"", "error", "$undefined", "\"<Language: XPath2>\"",
  "\"<Language: XQuery>\"", "\"<Language: XSLT 2.0>\"",
  "\"<Language: Function Declaration>\"", "\"<Language: Delayed Module>\"",
  "\"<Language: Function Signature>\"", "\"$\"", "\":=\"", "\"*\"",
  "\"|\"", "\"!=\"", "\"<=\"", "\"> (comparison operator)\"", "\">=\"",
  "\"<<\"", "\">>\"", "\"::\"", "\"@\"", "\"[\"", "\"]\"", "\"?\"",
  "\"<\"", "\"> (start tag close)\"", "\"> (end tag close)\"", "\"<?\"",
  "\"<!--\"", "\"-->\"", "\"//\"", "\"</\"", "\"(#\"", "\",\"", "\"+\"",
  "\"-\"", "\"/\"", "\"= (comparison operator)\"", "\".\"", "\"..\"",
  "\"(\"", "\")\"", "\"/>\"", "\"=\"", "\"\\\" (open)\"",
  "\"\\\" (close)\"", "\"' (open)\"", "\"' (close)\"", "\"{\"", "\"}\"",
  "\";\"", "\"#\"", "\"<integer literal>\"", "\"<decimal literal>\"",
  "\"<double literal>\"", "\"attribute\"", "\"comment\"",
  "\"document-node\"", "\"namespace-node\"", "\"node\"",
  "\"processing-instruction\"", "\"schema-attribute\"",
  "\"schema-element\"", "\"text\"", "\"empty-sequence\"",
  "\"boundary-space\"", "\"ft-option\"", "\"base-uri\"", "\"lax\"",
  "\"strict\"", "\"idiv\"", "\"child\"", "\"descendant\"", "\"self\"",
  "\"descendant-or-self\"", "\"following-sibling\"", "\"following\"",
  "\"parent\"", "\"ancestor\"", "\"preceding-sibling\"", "\"preceding\"",
  "\"ancestor-of-self\"", "\"document\"", "\"not\"", "\"using\"", "\"no\"",
  "\"sensitive\"", "\"insensitive\"", "\"diacritics\"", "\"without\"",
  "\"without (followed by content)\"", "\"stemming\"", "\"thesaurus\"",
  "\"stop\"", "\"wildcards\"", "\"entire\"", "\"content\"", "\"word\"",
  "\"type\"", "\"start\"", "\"end\"", "\"most\"", "\"skip\"", "\"copy\"",
  "\"value\"", "\"<whitespace>\"", "\"<processing instruction content>\"",
  "\"<xml comment content>\"", "\"eq\"", "\"ne\"", "\"lt\"", "\"le\"",
  "\"gt\"", "\"ge\"", "\"at\"", "\"<quot attribute content>\"",
  "\"<apos attribute content>\"", "\"<whitespace element content>\"",
  "\"<element content>\"", "\"at (followed by least or most)\"",
  "\"ordered\"", "\"unordered\"", "\"<qualified name>\"",
  "\"<xmlns qualified name>\"",
  "\"<computed constructor qualified name>\"", "\"'...'\"", "\"variable\"",
  "\"<NCName>:*\"", "\"*:<NCName>\"",
  "\"<processing instruction target>\"", "\"<pragma content>\"",
  "\"return\"", "\"for\"", "\"in\"", "\"let\"", "\"where\"", "\"count\"",
  "\"by\"", "\"order\"", "\"stable\"", "\"ascending\"", "\"descending\"",
  "\"empty\"", "\"greatest\"", "\"least\"", "\"collation\"", "\"some\"",
  "\"every\"", "\"satisfies\"", "\"typeswitch\"", "\"case\"",
  "\"case (followed by (in)sensitive)\"", "\"as\"", "\"if\"", "\"then\"",
  "\"else\"", "\"or\"", "\"and\"", "\"instance\"", "\"of\"",
  "\"castable\"", "\"to\"", "\"div\"", "\"mod\"", "\"union\"",
  "\"intersect\"", "\"except\"", "\"validate\"", "\"cast\"", "\"treat\"",
  "\"is\"", "\"preserve\"", "\"strip\"", "\"namespace\"", "\"item\"",
  "\"external\"", "\"encoding\"", "\"no-preserve\"", "\"inherit\"",
  "\"no-inherit\"", "\"declare\"", "\"construction\"", "\"ordering\"",
  "\"default\"", "\"copy-namespaces\"", "\"option\"", "\"xquery\"",
  "\"version\"", "\"import\"", "\"schema\"", "\"module\"", "\"element\"",
  "\"function\"", "\"function (ext)\"", "\"score\"", "\"contains\"",
  "\"weight\"", "\"window\"", "\"distance\"", "\"occurs\"", "\"times\"",
  "\"same\"", "\"different\"", "\"lowercase\"", "\"uppercase\"",
  "\"relationship\"", "\"levels\"", "\"language\"", "\"any\"", "\"all\"",
  "\"phrase\"", "\"exactly\"", "\"from\"", "\"words\"", "\"sentences\"",
  "\"paragraphs\"", "\"sentence\"", "\"paragraph\"", "\"replace\"",
  "\"modify\"", "\"first\"", "\"insert\"", "\"before\"", "\"after\"",
  "\"revalidation\"", "\"with\"", "\"nodes\"", "\"rename\"", "\"last\"",
  "\"delete\"", "\"into\"", "\"updating\"", "\"id\"", "\"key\"",
  "\"template\"", "\"matches\"", "\"name\"", "\"call\"", "\"apply\"",
  "\"templates\"", "\"mode\"", "\"ftor\"", "\"ftand\"", "\"ftnot\"",
  "\"private\"", "\"public\"", "\"deterministic\"", "\"nondeterministic\"",
  "\"<XSLT end element>\"", "\"<xsl:stylesheet...\"",
  "\"<xsl:template...\"", "\"<xsl:value-of...\"", "\"<xsl:text...\"",
  "\"<xsl:apply-templates...\"", "\"<xsl:call-template...\"",
  "\"<xsl:with-param...\"", "\"<xsl:sequence...\"", "\"<xsl:param...\"",
  "\"<xsl:function...\"", "\"<xsl:choose...\"", "\"<xsl:when...\"",
  "\"<xsl:otherwise...\"", "\"<xsl:if...\"", "\"<xsl:variable...\"",
  "\"<xsl:comment...\"", "\"<xsl:processing-instruction...\"",
  "\"<xsl:document...\"", "\"<xsl:attribute...\"", "\"<xsl:namespace...\"",
  "\"<xsl:element...\"", "\"<xsl:analyze-string...\"",
  "\"<xsl:matching-substring...\"", "\"<xsl:non-matching-substring...\"",
  "\"<xsl:copy-of...\"", "\"<xsl:copy...\"", "\"<xsl:for-each...\"",
  "\"<xsl:output...\"", "\"<xsl:import-schema...\"", "\"version='...'\"",
  "\"mode='...'\"", "\"name='...'\"", "\"doctype-public='...'\"",
  "\"doctype-system='...'\"", "\"encoding='...'\"", "\"media-type='...'\"",
  "\"normalization-form='...'\"", "\"standalone='...'\"",
  "\"exclude-result-prefixes='...'\"", "\"namespace='...' (string)\"",
  "\"schema-location='...'\"", "\"tunnel='...'\"", "\"required='...'\"",
  "\"override='...'\"", "\"copy-namespaces='...'\"",
  "\"inherit-namespaces='...'\"", "\"byte-order-mark='...'\"",
  "\"escape-uri-attributes='...'\"", "\"include-content-type='...'\"",
  "\"indent='...'\"", "\"omit-xml-declaration='...'\"",
  "\"undeclare-prefixes='...'\"", "\"match='...'\"", "\"as='...'\"",
  "\"select='...'\"", "\"priority='...'\"", "\"test='...'\"",
  "\"separator='...'\"", "\"namespace='...' (attribute value template)\"",
  "\"regex='...'\"", "\"flags='...'\"", "\"method='...'\"",
  "\"cdata-section-elements='...'\"", "\"use-character-maps='...'\"",
  "\"<XSLT element name>\"", "\"<XSLT XMLNS attr>\"",
  "\"<XSLT attr name>\"", "\"<XSLT text node>\"",
  "\"<XSLT whitespace text node>\"", "\"#default\"", "\"#all\"",
  "\"#current\"", "\"xml\"", "\"html\"", "\"xhtml\"", "$accept",
  "SelectLanguage", "XPathBuiltinNamespaces", "XQueryBuiltinNamespaces",
  "Start_FunctionDecl", "FunctionDecl_MaybeSemicolon",
  "Start_FunctionSignature", "Start_DelayedModule", "DM_ModuleDecl",
  "DM_Prolog", "DM_FunctionDecl", "Start_XSLT", "Stylesheet_XSLT",
  "StylesheetAttrs_XSLT", "StylesheetContent_XSLT", "Template_XSLT",
  "TemplateAttrs_XSLT", "TemplateModes_XSLT", "Number_XSLT",
  "Function_XSLT", "FunctionAttrs_XSLT", "ParamList_XSLT", "Param_XSLT",
  "ParamAttrs_XSLT", "GlobalParam_XSLT", "GlobalParamAttrs_XSLT",
  "GlobalVariable_XSLT", "GlobalVariableAttrs_XSLT", "Output_XSLT",
  "OutputAttrs_XSLT", "OutputMethod_XSLT", "QNames_XSLT",
  "ImportSchema_XSLT", "ImportSchemaAttrs_XSLT",
  "LiteralResultElement_XSLT", "LiteralResultElementAttrs_XSLT",
  "AttrValueTemplate_XSLT", "SequenceConstructor_XSLT", "Instruction_XSLT",
  "ValueOf_XSLT", "ValueOfAttrs_XSLT", "Text_XSLT", "TextNode_XSLT",
  "ApplyTemplates_XSLT", "ApplyTemplatesAttrs_XSLT",
  "ApplyTemplatesMode_XSLT", "ApplyTemplatesContent_XSLT",
  "CallTemplate_XSLT", "CallTemplateAttrs_XSLT",
  "CallTemplateContent_XSLT", "WithParam_XSLT", "WithParamAttrs_XSLT",
  "Sequence_XSLT", "SequenceAttrs_XSLT", "If_XSLT", "IfAttrs_XSLT",
  "Choose_XSLT", "WhenList_XSLT", "When_XSLT", "WhenAttrs_XSLT",
  "Otherwise_XSLT", "AnalyzeString_XSLT", "AnalyzeStringAttrs_XSLT",
  "MatchingSubstring_XSLT", "NonMatchingSubstring_XSLT", "Variable_XSLT",
  "VariableAttrs_XSLT", "Comment_XSLT", "CommentAttrs_XSLT", "PI_XSLT",
  "PIAttrs_XSLT", "Document_XSLT", "DocumentAttrs_XSLT", "Attribute_XSLT",
  "AttributeAttrs_XSLT", "Namespace_XSLT", "NamespaceAttrs_XSLT",
  "Element_XSLT", "ElementAttrs_XSLT", "CopyOf_XSLT", "CopyOfAttrs_XSLT",
  "Copy_XSLT", "CopyAttrs_XSLT", "ForEach_XSLT", "ForEachAttrs_XSLT",
  "Pattern_XSLT", "PathPattern_XSLT", "PathPatternStart_XSLT",
  "IdKeyPattern_XSLT", "IdValue_XSLT", "KeyValue_XSLT",
  "RelativePathPattern_XSLT", "PatternStep_XSLT", "PatternAxis_XSLT",
  "PatternStepPredicateList_XSLT", "Module", "VersionDecl", "Version",
  "Encoding", "MainModule", "LibraryModule", "ModuleDecl", "Prolog",
  "Setter", "Import", "Separator", "NamespaceDecl", "BoundarySpaceDecl",
  "DefaultNamespaceDecl", "OptionDecl", "FTOptionDecl", "OrderingModeDecl",
  "EmptyOrderDecl", "CopyNamespacesDecl", "PreserveMode", "InheritMode",
  "DefaultCollationDecl", "BaseURIDecl", "SchemaImport",
  "ResourceLocations", "SchemaPrefix", "ModuleImport", "VarDecl",
  "VarDeclValue", "ConstructionDecl", "FunctionDecl", "FunctionKeyword",
  "FunctionParamList", "FunctionBody", "FunctionOptions", "PrivateOption",
  "DeterministicOption", "FunctionDeclReturnType", "ParamList", "Param",
  "EnclosedExpr", "QueryBody", "Expr", "ExprSingle", "FLWORExpr",
  "FLWORTuples", "InitialClause", "IntermediateClause", "ForClause",
  "ForBindingList", "ForBinding", "PositionalVar", "FTScoreVar",
  "LetClause", "LetBindingList", "LetBinding", "WhereClause",
  "CountClause", "OrderByClause", "OrderSpecList", "OrderSpec",
  "OrderExpr", "OrderDirection", "EmptyHandling", "QuantifiedExpr",
  "QuantifyBindingList", "QuantifyBinding", "TypeswitchExpr",
  "CaseClauseList", "DefaultCase", "CaseClause", "CaseSequenceTypeUnion",
  "CaseClauseVariable", "IfExpr", "OrExpr", "AndExpr", "ComparisonExpr",
  "@1", "FTContainsExpr", "RangeExpr", "AdditiveExpr",
  "MultiplicativeExpr", "UnionExpr", "IntersectExceptExpr",
  "InstanceofExpr", "TreatExpr", "CastableExpr", "CastExpr", "UnaryExpr",
  "ValueExpr", "ValidateExpr", "ExtensionExpr", "PragmaList", "Pragma",
  "PragmaContents", "PathExpr", "LeadingSlash", "RelativePathExpr",
  "StepExpr", "AxisStep", "ForwardStepPredicateList",
  "ReverseStepPredicateList", "ForwardStep", "ForwardAxis",
  "AbbrevForwardStep", "ReverseStep", "ReverseAxis", "AbbrevReverseStep",
  "NodeTest", "NameTest", "Wildcard", "PostfixExpr", "PrimaryExpr",
  "Literal", "NumericLiteral", "VarRef", "VarName", "ParenthesizedExpr",
  "ContextItemExpr", "OrderedExpr", "UnorderedExpr", "FunctionCall",
  "FunctionCallArgumentList", "Argument", "Constructor",
  "DirectConstructor", "DirElemConstructor", "DirElemConstructorQName",
  "DirAttributeList", "OptionalWhitespace", "DirAttributeValue",
  "LiteralDirAttributeValue", "QuotAttrValueContent",
  "LiteralQuotAttrValueContent", "AposAttrValueContent",
  "LiteralAposAttrValueContent", "DirElementContent",
  "DirCommentConstructor", "DirCommentContents", "DirPIConstructor",
  "DirPIContents", "ComputedConstructor", "CompDocConstructor",
  "CompElemConstructor", "CompElementName", "ContentExpr",
  "CompAttrConstructor", "CompAttrName", "CompNamespaceConstructor",
  "CompTextConstructor", "CompCommentConstructor", "CompPIConstructor",
  "CompPINCName", "CompPIConstructorContent", "SingleType",
  "SingleTypeOccurrence", "TypeDeclaration", "SequenceType",
  "OccurrenceIndicator", "ItemType", "AtomicType", "KindTest",
  "AnyKindTest", "DocumentTest", "TextTest", "CommentTest",
  "NamespaceNodeTest", "PITest", "AttributeTest", "AttribNameOrWildcard",
  "SchemaAttributeTest", "AttributeDeclaration", "ElementTest",
  "ElementNameOrWildcard", "SchemaElementTest", "ElementDeclaration",
  "AttributeName", "ElementName", "TypeName", "URILiteral", "FTSelection",
  "FTSelectionWeight", "FTPosFilters", "FTOr", "FTAnd", "FTMildnot",
  "FTUnaryNot", "FTPrimaryWithOptions", "FTPrimary", "FTWords",
  "FTExtensionSelection", "FTAnyallOption", "FTTimes", "FTRange",
  "FTPosFilter", "FTUnit", "FTBigUnit", "FTMatchOptions", "FTMatchOption",
  "FTCaseOption", "FTDiacriticsOption", "FTStemOption",
  "FTThesaurusOption", "FTThesaurusIDList", "FTThesaurusID",
  "FTStopWordOption", "FTStopWordsInclExclList", "FTStopWords",
  "FTStopWordsStringList", "FTStopWordsInclExcl", "FTLanguageOption",
  "FTWildCardOption", "FTExtensionOption", "FTIgnoreOption",
  "RevalidationDecl", "InsertExpr", "InsertExprBegin", "DeleteExpr",
  "DeleteExprBegin", "ReplaceExpr", "RenameExpr", "TransformExpr",
  "TransformBindingList", "TransformBinding", "IntegerLiteral",
  "DecimalLiteral", "DoubleLiteral", "StringLiteral", "NCName", "QName",
  "TemplateDecl", "TemplateSequenceType", "TemplateParamList",
  "TemplateDeclModesSection", "TemplateDeclModes", "TemplateDeclMode",
  "CallTemplateExpr", "ApplyTemplatesExpr", "ApplyTemplatesMode",
  "TemplateArgumentList", "TemplateArgument", "FunctionItemExpr",
  "LiteralFunctionItem", "InlineFunction", "DynamicFunctionInvocation",
  "FunctionTest", "AnyFunctionTest", "TypedFunctionTest",
  "FunctionTypeArguments", "ParenthesizedItemType", "QNameValue",
  "FunctionName", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,   509,   510,   511,   512,   513,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   530,   531,   532,   533,   534,
     535,   536,   537,   538,   539,   540,   541,   542,   543,   544,
     545,   546,   547,   548,   549,   550,   551,   552,   553,   554,
     555,   556,   557,   558,   559,   560,   561,   562,   563,   564,
     565,   566,   567,   568,   569,   570,   571,   572,   573,   574,
     575,   576,   577,   578
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   324,   325,   325,   325,   325,   325,   325,   326,   327,
     328,   329,   329,   330,   331,   332,   333,   333,   334,   335,
     336,   336,   337,   337,   337,   338,   338,   338,   338,   338,
     338,   338,   339,   340,   340,   340,   340,   340,   340,   341,
     341,   341,   341,   342,   342,   342,   343,   344,   344,   344,
     344,   345,   345,   346,   347,   347,   347,   347,   347,   347,
     348,   349,   349,   349,   349,   349,   349,   350,   351,   351,
     351,   351,   352,   353,   353,   353,   353,   353,   353,   353,
     353,   353,   353,   353,   353,   353,   353,   353,   353,   353,
     353,   354,   354,   354,   354,   354,   355,   355,   356,   357,
     357,   357,   357,   357,   358,   359,   359,   359,   360,   360,
     360,   361,   361,   361,   361,   362,   362,   362,   362,   362,
     362,   362,   362,   362,   362,   362,   362,   362,   362,   362,
     362,   362,   362,   363,   364,   364,   364,   365,   366,   366,
     367,   368,   368,   368,   369,   369,   369,   370,   370,   371,
     372,   372,   373,   373,   374,   375,   375,   375,   375,   375,
     376,   377,   378,   379,   380,   381,   381,   382,   383,   384,
     384,   385,   386,   386,   386,   386,   387,   387,   388,   388,
     389,   390,   390,   390,   390,   391,   392,   392,   393,   394,
     394,   394,   395,   396,   397,   398,   398,   398,   398,   398,
     399,   400,   400,   400,   401,   402,   402,   402,   403,   404,
     404,   404,   405,   406,   406,   406,   407,   408,   408,   409,
     409,   410,   410,   410,   411,   411,   412,   412,   413,   413,
     414,   414,   415,   415,   415,   415,   415,   415,   416,   417,
     417,   417,   417,   418,   418,   419,   419,   419,   419,   420,
     420,   420,   421,   422,   423,   424,   425,   426,   426,   426,
     426,   426,   426,   426,   426,   426,   426,   427,   427,   427,
     427,   427,   427,   427,   427,   428,   428,   429,   430,   431,
     431,   432,   432,   433,   434,   435,   435,   436,   436,   437,
     438,   438,   439,   439,   440,   441,   442,   442,   443,   443,
     443,   444,   444,   445,   445,   446,   447,   447,   448,   448,
     449,   450,   450,   451,   451,   452,   452,   453,   453,   453,
     453,   454,   454,   455,   455,   456,   456,   457,   457,   458,
     459,   460,   461,   461,   462,   462,   462,   462,   462,   462,
     462,   462,   462,   462,   462,   462,   463,   464,   464,   464,
     465,   465,   466,   466,   466,   467,   468,   468,   469,   470,
     470,   471,   471,   472,   473,   473,   474,   474,   475,   476,
     477,   477,   478,   478,   479,   479,   480,   481,   481,   481,
     482,   482,   482,   483,   483,   484,   484,   485,   486,   487,
     487,   488,   488,   489,   490,   490,   491,   491,   492,   493,
     493,   494,   494,   495,   495,   496,   495,   495,   495,   495,
     495,   495,   495,   495,   495,   495,   495,   495,   495,   495,
     497,   497,   497,   498,   498,   499,   499,   499,   500,   500,
     500,   500,   500,   501,   501,   501,   502,   502,   502,   503,
     503,   504,   504,   505,   505,   506,   506,   507,   507,   507,
     508,   508,   508,   509,   509,   509,   510,   510,   511,   511,
     512,   513,   513,   514,   514,   514,   514,   515,   516,   516,
     516,   517,   517,   518,   518,   519,   519,   520,   520,   521,
     521,   522,   522,   522,   522,   522,   522,   522,   522,   523,
     523,   524,   524,   525,   525,   525,   525,   525,   526,   527,
     527,   528,   528,   529,   529,   529,   530,   530,   530,   531,
     531,   531,   531,   531,   531,   531,   531,   531,   532,   532,
     533,   533,   533,   534,   535,   536,   536,   537,   538,   539,
     540,   540,   541,   541,   542,   542,   543,   543,   544,   544,
     544,   545,   545,   546,   546,   547,   547,   547,   548,   548,
     549,   549,   550,   550,   551,   551,   551,   552,   552,   552,
     553,   553,   553,   554,   554,   554,   555,   555,   555,   555,
     555,   556,   557,   557,   558,   559,   559,   560,   560,   560,
     560,   560,   560,   560,   561,   562,   563,   563,   564,   564,
     565,   566,   566,   567,   568,   569,   570,   571,   571,   572,
     572,   573,   574,   574,   575,   575,   576,   576,   577,   577,
     577,   577,   578,   578,   578,   578,   578,   579,   580,   580,
     580,   580,   580,   580,   580,   580,   580,   580,   581,   582,
     582,   582,   583,   584,   585,   586,   586,   586,   587,   587,
     587,   588,   588,   589,   590,   591,   591,   591,   591,   592,
     592,   593,   594,   595,   596,   597,   598,   599,   600,   600,
     601,   601,   602,   602,   603,   603,   604,   604,   605,   605,
     606,   606,   607,   607,   607,   608,   608,   609,   609,   610,
     610,   610,   610,   610,   610,   611,   611,   612,   612,   612,
     612,   613,   613,   613,   613,   613,   613,   613,   613,   614,
     614,   614,   615,   615,   616,   616,   617,   617,   617,   617,
     617,   617,   617,   617,   618,   618,   618,   618,   619,   619,
     620,   620,   621,   621,   621,   621,   621,   622,   622,   623,
     623,   623,   623,   624,   624,   624,   625,   625,   626,   626,
     627,   627,   628,   628,   629,   630,   630,   631,   632,   633,
     633,   633,   634,   634,   634,   634,   634,   635,   635,   636,
     637,   637,   638,   638,   639,   640,   641,   641,   642,   643,
     644,   645,   646,   647,   648,   649,   649,   649,   649,   649,
     649,   650,   650,   651,   651,   652,   652,   653,   653,   654,
     654,   654,   655,   655,   656,   656,   656,   656,   657,   657,
     657,   658,   658,   659,   660,   660,   661,   662,   663,   663,
     664,   664,   665,   666,   666,   667,   667,   668,   669,   669,
     669,   669,   669,   669,   669,   669,   669,   669,   669,   669,
     669,   669,   669,   669,   670,   670,   670,   670,   670,   670,
     670,   670,   670,   670,   670,   670,   670,   670,   670,   670,
     670,   670,   670,   670,   670,   670,   670,   670,   670,   670,
     670,   670,   670,   670,   670,   670,   670,   670,   670,   670,
     670,   670,   670,   670,   670,   670,   670,   670,   670,   670,
     670,   670,   670,   670,   670,   670,   670,   670,   670,   670,
     670,   670,   670,   670,   670,   670,   670,   670,   670,   670,
     670,   670,   670,   670,   670,   670,   670,   670,   670,   670,
     670,   670,   670,   670,   670,   670,   670,   670,   670,   670,
     670,   670,   670,   670,   670,   670,   670,   670,   670,   670,
     670,   670,   670,   670,   670,   670,   670,   670,   670,   670,
     670,   670,   670,   670,   670,   670,   670,   670,   670,   670,
     670,   670,   670,   670,   670,   670,   670,   670,   670,   670,
     670,   670,   670,   670,   670,   670,   670,   670,   670,   670,
     670,   670,   670,   670,   670,   670,   670,   670,   670,   670,
     670,   670,   670,   670,   670,   670,   670,   670,   670,   670,
     670,   670
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     3,     3,     2,     2,     2,     0,     0,
       8,     0,     1,     3,     2,     6,     0,     2,     8,     1,
       4,     1,     0,     2,     2,     0,     2,     2,     2,     2,
       2,     2,     4,     1,     3,     2,     3,     3,     3,     0,
       2,     2,     2,     1,     1,     1,     4,     1,     2,     3,
       2,     0,     2,     3,     1,     2,     3,     3,     2,     2,
       3,     1,     2,     3,     3,     2,     2,     3,     1,     2,
       3,     3,     2,     1,     2,     3,     2,     3,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     3,
       2,     1,     1,     1,     1,     1,     1,     2,     2,     1,
       2,     2,     3,     3,     4,     0,     3,     2,     0,     2,
       2,     0,     2,     2,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     3,     3,     3,     1,     1,
       3,     1,     3,     3,     1,     1,     1,     0,     2,     3,
       1,     2,     0,     2,     3,     1,     2,     3,     3,     2,
       2,     3,     3,     3,     4,     1,     2,     3,     3,     0,
       3,     4,     1,     3,     3,     3,     0,     3,     0,     3,
       4,     1,     2,     3,     3,     3,     1,     3,     3,     1,
       3,     3,     3,     1,     3,     1,     3,     3,     3,     3,
       3,     1,     3,     3,     3,     1,     3,     3,     2,     1,
       3,     2,     3,     1,     2,     2,     3,     1,     3,     3,
       1,     1,     1,     1,     1,     1,     4,     6,     1,     1,
       1,     1,     1,     2,     3,     3,     3,     3,     2,     1,
       3,     3,     2,     0,     4,     2,     2,     1,     1,     3,
       4,     3,     2,     2,     2,     2,     6,     0,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     5,     3,
       3,     5,     5,     4,     3,     3,     3,     5,     5,     5,
       1,     1,     1,     1,     4,     3,     5,     4,     0,     2,
       3,     3,     3,     7,     4,     6,     2,     1,     3,     3,
       7,     1,     1,     2,     3,     1,     1,     0,     2,     2,
       2,     1,     1,     1,     1,     0,     2,     3,     1,     3,
       3,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     2,     2,
       1,     1,     1,     1,     1,     2,     3,     1,     7,     0,
       3,     0,     3,     2,     3,     1,     5,     5,     2,     3,
       3,     4,     3,     1,     3,     5,     1,     0,     1,     1,
       0,     2,     2,     4,     4,     3,     1,     5,     6,     2,
       1,     5,     3,     3,     3,     3,     0,     3,     8,     3,
       1,     3,     1,     3,     3,     0,     4,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     1,
       4,     5,     1,     3,     1,     3,     3,     1,     3,     3,
       3,     3,     1,     3,     3,     1,     3,     3,     1,     4,
       1,     4,     1,     4,     1,     4,     1,     2,     2,     1,
       1,     1,     1,     4,     5,     5,     3,     4,     1,     2,
       4,     1,     2,     1,     2,     2,     1,     1,     3,     3,
       1,     1,     1,     1,     1,     1,     4,     1,     4,     2,
       1,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       1,     2,     1,     2,     2,     2,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     1,     3,     2,     1,     4,     4,
       3,     4,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     5,    10,     1,     1,     0,     7,     7,     0,     1,
       3,     3,     3,     3,     0,     2,     2,     0,     2,     2,
       0,     2,     2,     0,     2,     2,     0,     2,     2,     2,
       2,     3,     0,     1,     3,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     4,     3,     1,     3,     2,     3,
       3,     1,     3,     3,     4,     4,     3,     1,     3,     2,
       3,     2,     0,     1,     0,     2,     2,     3,     0,     1,
       1,     1,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     3,
       4,     4,     3,     3,     3,     3,     4,     4,     3,     4,
       6,     1,     1,     4,     1,     3,     4,     6,     7,     1,
       1,     4,     1,     1,     1,     1,     1,     3,     0,     2,
       0,     2,     3,     1,     3,     1,     4,     1,     2,     1,
       1,     2,     2,     3,     1,     2,     4,     3,     4,     0,
       1,     2,     1,     2,     1,     0,     3,     2,     3,     3,
       4,     1,     3,     3,     2,     2,     2,     2,     2,     1,
       1,     1,     1,     1,     2,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     1,     1,     2,     2,
       1,     2,     2,     2,     5,     5,     2,     0,     3,     2,
       4,     4,     6,     4,     3,     4,     0,     2,     2,     3,
       1,     3,     2,     2,     2,     1,     2,     3,     3,     3,
       3,     3,     6,     6,     4,     4,     4,     2,     2,     2,
       2,     2,     7,     5,     5,     6,     3,     1,     4,     1,
       1,     1,     1,     1,     1,     5,     7,     8,    10,     6,
       8,     0,     2,     2,     3,     0,     2,     1,     3,     1,
       1,     1,     3,     7,     3,     7,     5,     9,     1,     1,
       1,     1,     3,     5,     1,     1,     3,     4,     3,     4,
       1,     1,     4,     5,     6,     1,     3,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     8,     9,     0,     9,     0,   317,     0,     0,   257,
      22,   105,     5,    19,    21,     0,     0,     6,    16,     7,
       0,     1,     0,   503,     0,     0,     0,   572,     0,   548,
       0,     0,   467,   527,   498,     0,   769,   770,   771,   819,
     820,   821,   833,   825,   826,   827,   828,   829,   831,   835,
     836,   837,   838,   839,   840,   841,   842,   843,   844,   845,
     846,   847,   848,   849,   850,   851,   852,   853,   935,   936,
     854,   855,   856,   857,   858,   859,   860,   861,   862,   863,
     864,   865,   866,   867,   868,   869,   870,   871,   872,   873,
     874,   875,   876,   877,   878,   974,   975,   834,   772,   879,
     504,   505,   880,   881,   882,   883,   884,   885,   886,   887,
     888,   889,   890,   891,   892,   893,   894,   895,   896,   830,
     897,   898,   824,   899,   900,   901,   902,   903,   904,   905,
     906,   907,   908,   909,   910,   911,   912,   913,   914,   915,
     916,   917,   918,   823,   919,   920,   921,   922,   923,   924,
     925,   926,   927,   928,   929,   930,   931,   932,   822,   933,
     832,   934,   937,   938,   939,   940,   941,   942,   943,   944,
     945,   946,   947,   948,   949,   950,   951,   952,   953,   954,
     955,   956,   957,   958,   959,   960,   961,   962,   963,   964,
     965,   966,   967,   968,   969,   970,   971,   972,   973,   976,
     977,   978,   979,   980,   981,   982,   983,   984,   985,   986,
     987,   988,   989,   990,   991,     2,   331,   333,   334,     0,
     347,   350,   351,   335,   336,   337,   343,   400,   402,   419,
     422,   424,   427,   432,   435,   438,   440,   442,   444,   446,
     449,   450,   452,     0,   458,   451,   463,   466,   470,   471,
     473,   474,   475,     0,   480,   477,     0,   492,   490,   500,
     502,   472,   506,   509,   518,   510,   511,   512,   515,   516,
     513,   514,   536,   538,   539,   540,   537,   577,   578,   579,
     580,   581,   582,   583,   499,   627,   618,   625,   624,   626,
     623,   620,   622,   619,   621,   338,     0,   339,     0,   341,
     340,   342,   520,   521,   522,   519,   501,   344,   345,   517,
     804,   805,   508,   774,   818,     0,     0,     3,   257,   247,
     248,   257,     0,    25,   111,   317,     4,     0,    14,     0,
     320,   321,   322,   323,   324,   325,   318,   319,   819,   820,
     821,   833,   825,   826,   827,   828,   829,   841,   842,   843,
     844,   845,   846,   847,   848,   849,   850,   851,   852,   870,
     974,   975,   881,   883,   894,   895,   830,   824,   912,   918,
     822,   832,   960,   963,   969,   971,   981,   982,   523,   524,
     818,   819,   820,   826,   829,   822,   489,   543,   544,   545,
       0,   573,     0,   465,   549,     0,   448,   447,   526,     0,
     483,     0,     0,   591,     0,     0,     0,     0,     0,     0,
       0,     0,   597,     0,     0,     0,     0,     0,   481,   482,
     484,   485,   486,   487,   493,   494,   495,   496,   497,     0,
       0,     0,   767,     0,     0,     0,   355,   357,     0,     0,
     363,   365,     0,     0,   386,     0,     0,     0,     0,     0,
       0,   488,     0,     0,     0,   586,     0,   604,     0,     0,
     757,   758,     0,   760,   761,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   352,   349,   353,   354,   348,
       0,     0,     0,     0,     0,     0,     0,     0,   405,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   459,   464,     0,     0,     0,     0,
     479,   491,     0,     0,     0,   759,     0,     0,     0,     0,
       0,     0,     0,   245,   246,   255,   924,   931,     0,     0,
       0,   267,     0,     0,     0,   271,   272,   274,   268,   269,
     275,   276,     0,   270,     0,   254,   273,     0,    23,    24,
       0,   107,   108,     0,     0,     0,   773,   317,    17,     0,
     313,     0,   328,     0,    13,   548,     0,   575,   574,   571,
       0,   525,   642,   638,     0,   641,   653,     0,     0,   590,
     633,     0,   629,     0,     0,     0,     0,   634,   628,   635,
       0,     0,     0,     0,   596,     0,   644,     0,   652,   654,
     632,     0,     0,     0,     0,     0,     0,     0,   604,     0,
     604,     0,     0,   604,     0,     0,     0,     0,     0,     0,
       0,     0,   593,   650,   645,     0,   649,     0,   585,     0,
       0,     0,     0,     0,   792,   794,   332,   346,   368,     0,
       0,     0,   399,   401,   404,   407,   408,   409,   417,   418,
       0,   403,   410,   411,   412,   413,   414,   415,   416,     0,
     425,   426,   423,   428,   430,   429,   431,   433,   434,   436,
     437,     0,     0,     0,     0,   456,     0,   469,   468,     0,
       0,     0,   535,   808,   534,     0,   533,     0,     0,     0,
       0,   530,     0,   806,   253,   252,   277,     0,   249,   251,
       0,   317,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   785,     0,     0,     0,   258,   259,
     260,   261,   265,   266,   262,   263,   264,    20,    33,    61,
      47,    68,    73,    99,    26,    51,    27,    51,    28,   111,
      29,   111,    30,     0,    31,     0,   106,   104,   134,     0,
     141,   150,     0,     0,     0,   181,   186,   189,   193,   195,
     201,   205,   172,   209,   213,   217,   138,   139,   113,   114,
     115,   111,   116,   112,   117,   147,   118,   152,   119,     0,
     121,   111,   120,   129,   176,   122,   111,   123,   111,   124,
     111,   125,   111,   126,   111,   127,   111,   128,   111,   130,
       0,   131,   111,   132,   111,   311,   312,     0,     0,     0,
     604,     0,   314,     0,   831,   823,   832,   326,   608,   612,
     614,   617,   615,   810,   811,   616,   549,     0,   576,     0,
     461,   460,     0,   639,   592,   588,     0,   595,   630,   631,
     637,   636,   598,   599,     0,   643,   651,   594,   584,     0,
     766,     0,   528,   529,   359,   356,     0,     0,   364,     0,
     385,   383,   384,     0,     0,   453,     0,     0,     0,   646,
     587,   605,     0,   807,     0,     0,     0,     0,     0,     0,
     369,   376,   370,   373,   377,     0,   406,     0,     0,     0,
       0,   679,   420,   660,   663,   665,   667,   669,   670,   685,
     674,   439,   441,   443,   602,   445,   457,   476,   478,   507,
       0,   809,     0,     0,   756,   755,   754,   531,   250,     0,
     279,   280,     0,   284,   656,   295,     0,     0,   308,   309,
     285,   286,     0,     0,     0,     0,   290,   291,     0,     0,
     750,   749,   751,     0,     0,     0,     0,     0,     0,     0,
     298,     0,   298,   100,   101,    39,    35,     0,     0,     0,
     111,    48,    50,     0,   111,    62,    66,    65,     0,     0,
       0,    69,     0,     0,     0,    72,    90,    74,    78,    79,
      80,    84,    85,    87,    76,    81,    82,    83,    86,    88,
       0,     0,     0,    98,   110,   109,     0,     0,     0,   169,
     165,   111,     0,     0,   108,     0,     0,     0,     0,   151,
       0,   160,     0,   111,     0,   108,   108,   178,   182,     0,
       0,     0,     0,     0,   108,     0,     0,     0,   108,     0,
     108,   108,     0,   108,     0,     0,   108,   108,     0,   208,
     211,     0,   214,   215,     0,     0,     0,     0,     0,     0,
     329,   327,     0,     0,     0,     0,   609,   611,   610,   606,
     548,   548,   566,   541,   462,     0,   655,   589,   600,   768,
       0,     0,   361,     0,     0,     0,   396,     0,   390,     0,
       0,   454,   455,     0,     0,   763,     0,   764,     0,     0,
     799,   800,   796,   798,     0,   378,   379,   380,   371,     0,
       0,   668,     0,   680,   682,   684,   675,     0,   421,     0,
     658,     0,     0,   671,     0,   672,   603,   601,   532,     0,
       0,     0,     0,     0,   720,     0,     0,   745,     0,     0,
     716,   717,     0,   704,   710,   711,   709,   708,   712,   706,
     707,   713,     0,   604,     0,     0,   294,     0,     0,     0,
     283,   785,   789,   790,   791,   786,   787,     0,     0,     0,
       0,   298,     0,   297,     0,   304,   102,   103,    37,     0,
     225,   224,   819,   841,   976,   977,    34,   220,     0,   223,
     221,   232,   243,   239,    38,    43,    44,    45,    36,    54,
      52,   111,     0,    49,     0,    64,    63,    60,    71,    70,
      67,    94,    95,    91,    92,    93,    75,    96,    77,    89,
     137,   161,     0,   111,   166,     0,     0,   163,   135,   136,
     133,   144,   145,   146,   143,   142,   140,   155,   148,   111,
     149,   153,   162,     0,   173,   174,   175,   111,     0,   184,
     183,   111,   187,   185,   190,   191,   188,   192,   196,   198,
     199,   197,   194,   202,   203,   200,   206,   207,   204,   210,
     212,   218,   216,   325,    15,     0,   817,   607,   613,     0,
       0,   815,     0,     0,     0,     0,   640,   765,     0,     0,
       0,   366,   367,   387,     0,     0,     0,   388,   389,     0,
       0,     0,     0,   647,   330,     0,     0,     0,   801,     0,
       0,   372,     0,   374,   673,   679,   677,     0,   681,   683,
       0,   662,     0,     0,   691,     0,     0,     0,     0,     0,
     657,   661,   664,     0,     0,     0,     0,     0,   752,   753,
     256,   721,   726,     0,   746,   719,   718,     0,     0,   723,
     722,     0,   715,   714,     0,   744,   705,     0,   278,   287,
     288,   281,   282,   292,   293,   289,     0,   775,   781,     0,
       0,     0,   325,   301,   302,   296,   299,     0,     0,    40,
      41,    42,   242,     0,     0,     0,     0,     0,   233,     0,
       0,     0,     0,   238,    55,    59,    58,     0,     0,     0,
      32,    46,    97,   168,     0,   164,   167,   156,   159,     0,
       0,     0,   177,     0,   171,   180,     0,   325,   812,     0,
       0,     0,   548,   548,     0,   570,   569,   568,   567,   360,
       0,     0,     0,   394,     0,     0,   395,   393,     0,   648,
     762,   604,     0,   793,   795,     0,   381,   382,     0,   676,
     678,   748,   698,   696,   697,   659,     0,     0,   702,   703,
     694,   695,   666,     0,     0,   687,     0,   686,   734,   727,
     727,   729,     0,     0,   736,   736,   747,     0,   307,   305,
     783,     0,     0,     0,     0,   788,   779,   781,     0,   300,
     298,   241,   240,     0,   229,   228,     0,   219,   235,   234,
     237,   236,     0,    57,    56,    53,   170,   158,   157,   154,
     179,    11,     0,   813,   816,     0,     0,     0,   548,   362,
     358,   397,     0,   392,   398,     0,   802,     0,   375,   699,
     700,   701,   692,   693,   689,   688,     0,     0,     0,     0,
       0,     0,   740,   738,   735,   733,   306,   784,   782,   776,
       0,     0,   316,   310,   315,   303,   226,     0,     0,    10,
      12,     0,   814,   554,   560,   546,   557,   563,   547,     0,
       0,     0,   797,   690,     0,   725,   724,   730,   731,     0,
     739,     0,     0,   737,   777,   781,   780,     0,   230,   231,
     244,    18,     0,     0,     0,     0,   542,   391,   803,   728,
       0,   741,   742,   743,     0,   227,   550,   556,   555,   551,
     562,   561,   552,   559,   558,   553,   565,   564,   732,   778
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     7,     8,     9,   326,  1559,    19,    17,    18,   328,
     568,    12,    13,   323,   560,   744,   745,  1178,  1198,   746,
     747,   970,  1200,  1201,   748,   749,   750,   751,   752,   753,
    1216,  1218,   754,   755,   778,   324,   756,   563,   779,   780,
     781,   782,   783,   784,   785,  1234,  1018,   786,   787,  1020,
    1238,  1239,   788,   789,   790,   791,   792,  1009,  1010,  1011,
    1225,   793,   794,  1027,  1248,   795,   796,   797,   798,   799,
     800,   801,   802,   803,   804,   805,   806,   807,   808,   809,
     810,   811,   812,   813,   814,  1186,  1187,  1188,  1189,  1493,
    1587,  1190,  1191,  1192,  1393,   317,   318,   530,   531,   319,
     320,   321,   322,   538,   539,   708,   540,   541,   542,   543,
     544,   545,   546,   547,   948,  1365,   548,   549,   550,  1173,
     959,   551,   552,  1479,   553,   554,   817,   335,  1553,   725,
     336,   337,   574,   571,   572,  1005,   215,   216,   217,   218,
     219,   220,   476,   221,   436,   437,  1082,  1290,   222,   440,
     441,   477,   478,   479,   892,   893,   894,  1107,  1313,   223,
     443,   444,   224,  1087,  1297,  1088,  1089,  1295,   225,   226,
     227,   228,   660,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   841,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   378,   266,   267,   268,   269,   270,   695,   696,   271,
     272,   273,   389,   575,   395,  1565,  1568,  1592,  1594,  1593,
    1595,  1285,   274,   392,   275,   578,   276,   277,   278,   456,
     589,   279,   404,   280,   281,   282,   283,   413,   604,   913,
    1127,   640,   827,  1069,   828,   829,   284,   285,   286,   287,
     288,   289,   290,   291,   584,   292,   605,   293,   635,   294,
     607,   585,   608,  1075,   935,   902,  1330,  1120,   903,   904,
     905,   906,   907,   908,   909,   910,  1116,  1125,  1337,  1331,
    1532,  1460,   933,  1143,  1144,  1145,  1146,  1147,  1537,  1350,
    1148,  1544,  1475,  1541,  1583,  1149,  1150,  1151,  1118,   556,
     295,   296,   297,   298,   299,   300,   301,   431,   432,   302,
     303,   304,   305,   565,   306,   557,  1483,  1368,   955,  1165,
    1166,   307,   308,  1102,  1307,  1308,   309,   310,   311,   312,
     832,   833,   834,  1282,   835,   313,   314
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1397
static const yytype_int16 yypact[] =
{
     296, -1397, -1397,    17, -1397,    13, -1397,   213,  3391,   478,
   -1397, -1397, -1397, -1397, -1397,   187,   246, -1397, -1397, -1397,
       7, -1397,  6346, -1397,  4819,   569,   341,   439,  4108,   477,
    3630,  3630, -1397, -1397, -1397,  2196, -1397, -1397, -1397,    96,
     258,   570,   584,   632,    68,   665,   669,   350, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397,   579,   616,   710,   713,   718,
     722,   726,   746,   757,   761,   763,   742, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397,   787, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397,   750,   766, -1397, -1397, -1397,
   -1397, -1397, -1397,   806, -1397,    36, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397,   807,   807, -1397,   777,
   -1397, -1397,   778, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397,   283, -1397, -1397, -1397,
   -1397, -1397,   153, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,    80, -1397,
     779, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397,    75, -1397, -1397,    -4, -1397,
   -1397, -1397, -1397, -1397,   764, -1397,    -3, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397,   588,   586, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397,   793, -1397, -1397,   535,
   -1397, -1397, -1397, -1397, -1397, -1397,   671,   673, -1397,  1091,
     636,    39,    52,    54,   554, -1397,   675,   666,   674,   668,
   -1397, -1397, -1397,   115, -1397, -1397,  4108,   305, -1397, -1397,
     816,   819, -1397,  4819, -1397, -1397,  4819, -1397, -1397, -1397,
   -1397,   157, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397,  3391, -1397,  3391, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397,   427,   330,   670, -1397,   651, -1397,
   -1397, -1397,  3869,   155,   407, -1397, -1397,  6346,   662,   158,
   -1397, -1397, -1397, -1397, -1397,   692, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397,   805,   808,   809,   813,   814, -1397, -1397, -1397, -1397,
     618, -1397,   817,   305, -1397,  6346, -1397, -1397, -1397,   441,
   -1397,  5293,  3391, -1397,   799,   818,  3391,    28,   822,   823,
    6153,  3391, -1397,   810,  6346,  6346,   824,  3391, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,  3391,
    6346,    20, -1397,  3391,  3391,  6346,   828, -1397,  6346,   846,
     833, -1397,  6346,    97, -1397,   125,  3391,  3391,  3391,   821,
     825, -1397,   799,  5530,  3391, -1397,   799,   705,  3391,   711,
   -1397, -1397,  3391, -1397, -1397,  6346,  3391,  3391,  3391,   806,
      36,  3391,   862,   737,   739, -1397, -1397, -1397, -1397, -1397,
    3630,  3630,  3630,  3630,  3630,  3630,  3630,  3630, -1397,  3630,
    3630,  3630,  3630,  3630,  3630,  3630,  3630,   812,  3630,  3630,
    3630,  3630,  3630,  3630,  3630,  3630,  3630,  3630,  3630,   719,
     727,   728,   729,  2435, -1397,   305,  4108,  4108,  3391,  3391,
   -1397, -1397,  3391,  1718,   -49, -1397,  1957,   830,   759,   760,
      50,   836,  6346, -1397, -1397,   323,  1036,   536,   836,   836,
     836, -1397,   836,   836,   836, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397,   836, -1397,   836, -1397, -1397,   836, -1397, -1397,
     106, -1397, -1397,   982,   194,   850, -1397, -1397, -1397,  6346,
   -1397,   496, -1397,  5738, -1397,   783,   784, -1397, -1397, -1397,
      27, -1397, -1397, -1397,   498, -1397, -1397,   260,  2674, -1397,
   -1397,   352, -1397,   669,   814,   848,   851, -1397, -1397, -1397,
     852,   853,   361,  2913, -1397,   856, -1397,   859, -1397, -1397,
   -1397,   366,   372,   892,   787,  3391,   411,   420,   705,   806,
     705,  6346,    36,   705,   807,  3391,  3391,   540,   546,   428,
    3391,  3391, -1397, -1397, -1397,   553, -1397,   431, -1397,  5738,
     855,   681,   849,   755,   687,   -87, -1397, -1397, -1397,  6346,
    3391,   772,   673, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
    3630, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,    45,
      52,    52,   693,    54,    54,    54,    54,   554,   554, -1397,
   -1397,  5738,  5738,  6346,  6346, -1397,   443, -1397, -1397,   502,
     510,   545, -1397, -1397, -1397,   555, -1397,   375,  3391,  3391,
    3391, -1397,   559, -1397, -1397, -1397, -1397,   836, -1397, -1397,
     874,   573,   536,   564,   829,   790,   907,  6346,   576,   619,
     204,   452,  6346,   221,   448,   194,   144,    18, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397,   461, -1397,   214, -1397,   170, -1397,   195,
   -1397,    34, -1397,  7057, -1397,   677,    44, -1397, -1397,   435,
   -1397, -1397,   614,   658,   621, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397,    74, -1397, -1397, -1397,    15, -1397,   643, -1397,   679,
   -1397, -1397, -1397, -1397,    67, -1397,   134, -1397,   626, -1397,
    -119, -1397, -1397, -1397,    65, -1397,   126, -1397,  -132, -1397,
    -117, -1397,   466, -1397,   627, -1397, -1397,  6529,   790,   194,
     705,   910, -1397,  5946,   881,   893,   896, -1397,   355, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397,   639,   349, -1397,   802,
   -1397, -1397,  6346, -1397, -1397, -1397,   446, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397,   455, -1397, -1397, -1397, -1397,  3391,
   -1397,   826, -1397, -1397,   835, -1397,   927,   930, -1397,   811,
   -1397, -1397, -1397,   800,   801, -1397,   493,   505,  6346, -1397,
   -1397, -1397,  3391, -1397,  3391,  3391,  3391,   913,   914,  1449,
   -1397, -1397,   931, -1397,   623,  3391, -1397,    45,  3391,   307,
     517,   488,   873,   724,   730,   885, -1397, -1397,   829,   771,
   -1397, -1397, -1397, -1397,   948, -1397, -1397, -1397, -1397, -1397,
    3152, -1397,   745,   748, -1397, -1397, -1397, -1397, -1397,   790,
   -1397, -1397,   527,   895, -1397, -1397,  6346,   939, -1397, -1397,
   -1397, -1397,   839,   790,   831,   832, -1397, -1397,   949,   858,
   -1397, -1397, -1397,  6346,   -70,   754,  6529,  6346,   794,   790,
     876,  6346,   876,   702,   704, -1397, -1397,  4345,  5738,   654,
     736, -1397, -1397,  5738,   736, -1397, -1397, -1397,  5738,  3391,
    1075, -1397,  5738,  3391,  1168, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
     -51,   877,   877, -1397, -1397, -1397,   747,  3391,   701,   506,
   -1397, -1397,  3391,  3391, -1397,  6530,   -93,  3391,   241, -1397,
     254, -1397,  6558, -1397,  3391, -1397, -1397,   738, -1397,  5738,
    3391,  6586,  3391,  6614, -1397,  3391,  6642,  6670, -1397,  3391,
   -1397, -1397,  6698, -1397,  3391,  6726, -1397, -1397,  6754, -1397,
   -1397,  3391, -1397, -1397,  6782,  3391,  6810,   779,   836,  6529,
   -1397, -1397,   957,   970,   974,  5056, -1397, -1397, -1397, -1397,
     477,   477, -1397, -1397, -1397,   975, -1397, -1397, -1397, -1397,
    3391,  1008,   827,  3391,  3391,  3391,  1010,   136, -1397,    64,
    3391, -1397, -1397,   292,   541, -1397,   795, -1397,  1013,  1013,
   -1397, -1397,   798, -1397,  3391, -1397, -1397,   882,   931,   986,
     544, -1397,    35,   932,   815, -1397, -1397,   934, -1397,    45,
     226,    45,   899,   895,    -1, -1397, -1397, -1397, -1397,  3391,
    3391,   836,   610,   682, -1397,    41,   834, -1397,   684,  6346,
   -1397, -1397,   909, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397,   527,   705,   790,   629, -1397,   790,   790,   596,
   -1397,     2, -1397, -1397, -1397,  1002, -1397,  4345,   779,  1004,
     869,   876,   790,  1018,  1016,  1018, -1397, -1397,   -58,  4819,
   -1397,   820,   268,  1035,  1015,  1017,  1046, -1397,  4582,   646,
     653, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397,   273,  6838, -1397,  6866, -1397,   793, -1397, -1397,   793,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,   938,   938,
   -1397,   793,  3391, -1397, -1397,   843,  6894,   793,   793,    44,
   -1397, -1397, -1397, -1397, -1397,   793, -1397, -1397, -1397,   281,
   -1397, -1397, -1397,  6922,   793,    44,    44, -1397,   864, -1397,
     793, -1397,   793, -1397,    44,   793, -1397, -1397,    44,   793,
      44,    44, -1397,    44,   793, -1397,    44,    44, -1397,   793,
   -1397,   793, -1397,   692, -1397,   779, -1397, -1397, -1397,  1020,
     911, -1397,   568,  1019,  1021,   118, -1397, -1397,  6346,  1056,
     933, -1397, -1397, -1397,  6346,  5738,    77, -1397, -1397,  5738,
    3391,   912,  1028, -1397, -1397,  3391,  6346,   572, -1397,   575,
    1030, -1397,   640,   935, -1397,   488, -1397,  1026, -1397, -1397,
    3630,   730,   985,   686, -1397,  3630,  3630,    -1,   571,   571,
   -1397, -1397,   885,    45,    25,  3630,  3630,   878, -1397, -1397,
   -1397, -1397, -1397,   883, -1397, -1397, -1397,    57,   790, -1397,
   -1397,    84, -1397, -1397,   972, -1397, -1397,    58, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397,   266, -1397,   946,   875,
     -70,   140,   692, -1397, -1397,  1018, -1397,   790,   790, -1397,
   -1397, -1397, -1397,  4819,  4819,    71,   909,  4345, -1397,  4582,
    4582,  4582,  4582,  1092, -1397, -1397, -1397,  5738,  3391,  6950,
   -1397, -1397, -1397,   793,  6978, -1397, -1397, -1397, -1397,  5738,
    3391,  7006, -1397,  7034, -1397,  1447,   855,   692, -1397,  5738,
    5738,   962,   477,   477,   569, -1397, -1397, -1397, -1397, -1397,
    6346,  3391,   966, -1397,  6346,  3391, -1397, -1397,  3391, -1397,
   -1397,   705,  1013, -1397, -1397,  1013, -1397, -1397,   790, -1397,
   -1397,    54, -1397, -1397, -1397, -1397,    70,   500, -1397, -1397,
   -1397, -1397, -1397,  3630,  3630,   693,    60, -1397, -1397, -1397,
   -1397,   -18,   909,   790, -1397, -1397, -1397,  3391, -1397, -1397,
   -1397,   589,  5738,   855,  4345, -1397, -1397,   946,    31, -1397,
     876, -1397, -1397,  1078, -1397, -1397,  1087, -1397, -1397, -1397,
   -1397, -1397,  3391, -1397,   793, -1397, -1397, -1397,   793, -1397,
   -1397,   836,   855, -1397, -1397,  5738,   358,   641,   477, -1397,
   -1397, -1397,   992, -1397, -1397,  1115, -1397,   592, -1397, -1397,
   -1397, -1397, -1397, -1397,   693,   693,  3630,   593,   595,   909,
     920,   608, -1397, -1397,   552,   552, -1397, -1397, -1397, -1397,
     140,   855, -1397, -1397, -1397,  1018, -1397,    87,   558, -1397,
   -1397,   836, -1397, -1397, -1397, -1397, -1397, -1397, -1397,  1101,
    3391,  3391, -1397,   693,  1024, -1397, -1397,    -1, -1397,   909,
   -1397,    69,    69, -1397, -1397,   946, -1397,  1088, -1397, -1397,
   -1397, -1397,    62,    66,   117,   249, -1397, -1397, -1397, -1397,
     924, -1397, -1397, -1397,   855, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1397, -1397, -1397,  1127, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397,   385, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397,   132, -1397, -1397,  1132, -1397,  -513,  -679, -1397, -1397,
   -1397, -1397,   380, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
     120, -1397, -1397, -1397, -1397, -1397, -1397, -1397,   133, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1150,  -246, -1397, -1397, -1397,
   -1397, -1397, -1110, -1397, -1397, -1397, -1397, -1397,   613,   838,
     841, -1397,   844, -1397, -1397,  -517, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,  -954,
   -1397, -1397, -1397, -1397, -1397, -1397,  -614,  -159, -1397,     3,
   -1397, -1397, -1208,  -220,   326,  -592,   842,     1,  -142, -1397,
   -1397,   929, -1397, -1397, -1397,   530, -1397, -1397, -1397, -1397,
     528, -1397, -1397, -1397,   256,    48, -1397, -1397, -1397, -1397,
    1037,   529, -1397, -1397, -1397,    73, -1397, -1397, -1397, -1397,
     688,   689, -1397,   161,  -158,  -497,   293,  -470,   288,   295,
   -1397, -1397, -1397,   774, -1397, -1397, -1397,  -653,  -237, -1397,
   -1397, -1397,    11,   294, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397,    -6, -1397, -1397, -1397, -1397,  -656, -1397,
   -1328,  -384, -1397, -1397, -1397, -1397, -1397,   645,   252, -1397,
    -116, -1397,  -250, -1397,  -571, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
     234, -1397, -1397, -1397, -1397, -1397, -1397,  1033, -1397,   492,
   -1397,  -613,  -620, -1397,   354,   129,  -423, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397,   773, -1397,   776,
   -1397,   765,   725,   303,  -667,  -856, -1397, -1397, -1397,    72,
      63,  -148,   287, -1397, -1397, -1397,  -128, -1397, -1303, -1397,
    -269,  -140,   282,    40, -1397, -1397, -1397, -1397,  -277, -1319,
   -1397,  -281,  -773, -1397, -1397, -1397, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397, -1397, -1397, -1397,   581, -1397,
   -1397, -1397, -1122,  -395,  -371, -1397, -1396, -1322,    37, -1397,
    -174, -1397, -1397, -1397, -1088,  -245, -1397, -1397, -1397, -1397,
   -1397, -1397, -1397, -1397, -1397,    16,   -22
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -318
static const yytype_int16 yytable[] =
{
     380,   457,   380,   672,   837,   864,   514,   866,  1175,    20,
     869,  1309,  1211,   901,   709,   601,   900,  1371,   386,   881,
    1355,   728,   729,   730,  1457,   731,   732,   733,  1470,  1231,
     586,   673,   674,   675,   676,   734,   399,   735,   379,   393,
     736,  1109,  1366,   586,   609,   438,   613,   329,   883,  1487,
     882,   618,  1162,   614,   620,   460,   463,  1494,   623,   960,
     962,   911,   912,   501,  1379,  1416,   505,    29,  1477,   592,
     980,  1212,   984,   498,   499,   897,  1299,    29,  1388,   882,
      22,  1347,   609,   898,  1316,   897,  1434,    36,    37,    38,
     593,  1551,   882,   898,   498,   499,    22,    36,    37,    38,
     706,  1334,  1015,   697,   498,   499,   945,  1606,   410,  1472,
     882,   956,  1022,  1609,   882,   400,   411,  1031,  1334,  1033,
     453,  1036,   502,  1037,  1472,  1042,  1463,  1045,   454,  1048,
     624,  1049,   839,  1054,   458,  1056,   401,   710,   888,    36,
      37,    38,    25,   934,   402,    26,    27,    29,  1046,  1424,
     830,  1058,  1387,   889,   524,  1348,   525,   840,   624,  1004,
      98,  1034,  1612,   513,  1488,   882,   882,   569,  1540,  1464,
      98,  1348,   451,   698,   699,  1047,  1050,  1607,   522,   459,
    1366,   700,  1610,  1473,  1035,   820,  1051,  1539,   882,  1604,
     928,   961,   412,  1335,  1336,  1300,    98,   523,  1473,   570,
     500,   411,   831,    16,   455,  1059,  1552,  1060,  1435,  1512,
    1335,  1336,    98,    21,   503,   504,   830,  1375,   506,   594,
     403,  1536,   461,   464,  1349,  1232,   528,  1233,  1585,  1589,
     439,   380,  1613,  1478,   380,  1425,  1426,   867,   330,   615,
    1469,   901,   954,   901,   900,   625,   900,   520,  1163,  1164,
     521,   331,   332,   333,   334,  1599,  1317,   515,   830,   830,
    1380,  1381,  1131,  1495,  1496,   890,    10,  1474,   831,   934,
    1213,  1214,  1215,   626,  1600,   569,  1156,   412,   899,  1498,
    1499,  1500,  1501,  1529,  1530,  1531,  1086,  1383,   899,   950,
     951,  1202,  1171,   467,  1016,  1204,  1615,   882,   405,     1,
       2,     3,     4,     5,     6,   380,   406,  1480,   401,   844,
     831,   831,   831,   831,   981,  1302,   641,   957,  1017,  1296,
     643,  1322,   937,   952,   645,   646,   647,   958,   564,   648,
      11,   448,  1226,  1303,  1550,   516,   982,   983,  1023,    29,
    1323,   517,   942,   566,  1243,  1038,  1324,   897,  1194,   943,
    1542,   449,   450,  1203,   737,   898,   738,  1527,  1205,    36,
      37,    38,  1208,   739,   740,  1616,  1066,   325,  1039,   741,
    1024,  1040,  1041,   380,  1072,  1025,  1026,  1013,  1067,   380,
    1014,   694,   742,   743,   694,   467,   815,   816,   380,  1068,
     416,  1073,   380,   380,   467,   944,   815,   816,   417,   467,
     830,   847,  1563,   587,  1564,   467,  1043,   591,   380,  1249,
     852,   580,   602,   380,  1028,   857,   380,  1577,   611,   327,
     380,   858,  1325,  1326,  1327,   330,   566,  1328,  1329,  1044,
     612,   380,    98,   558,   616,   617,  1029,  1030,   331,   332,
     333,   334,   559,   380,   467,  1281,   379,   627,   628,   629,
     971,   379,   831,   467,   379,   637,   901,  1601,   379,   900,
     862,   467,   972,   901,   467,   901,   900,   526,   900,   863,
     390,  1076,   973,   861,   467,   975,   467,   875,   527,   467,
     880,   644,   581,   871,   872,   976,   977,  1358,   467,  1236,
    1361,  1362,   916,   965,   966,  1077,  1237,   978,   979,  1283,
    1284,  1229,  1240,   711,  1078,  1376,   528,  1076,   891,  1237,
     380,   712,  1245,  1246,   686,   967,   968,   529,   969,   689,
     690,  1254,  1399,   691,   917,  1258,   467,  1260,  1261,   821,
    1263,   842,   918,  1266,  1267,   467,  1555,   822,   467,   843,
    1357,  1274,  1091,   467,  1404,   830,   391,   380,   566,    29,
     830,   380,  1153,  1394,  1092,   830,   924,   925,   926,   830,
    1411,  1407,  1169,  1395,  1396,  1112,  1174,   919,  1413,  1367,
     819,  1408,  1415,   467,   467,  1397,  1398,   467,   467,   467,
    1590,   873,   394,  1409,  1410,   379,   878,   874,   920,   846,
    1304,   467,   920,  1315,   879,   922,   921,   831,   418,   380,
     927,  1420,   831,   923,   854,  1442,   830,   831,  1442,  1421,
     407,   831,  1132,  1443,  1340,  1133,  1444,   380,  1134,  1135,
    1136,  1137,   821,   946,   408,  1442,  1574,   380,  1574,   947,
    1547,   876,   877,  1572,  1575,   419,  1576,   379,   713,   714,
     715,  1579,   830,   654,   655,   656,   657,   658,   659,  1580,
     661,   662,   663,   664,   665,   666,   667,   668,   831,   380,
     380,   380,   380,   514,   315,   379,   468,   469,   316,   470,
     471,   472,   409,   473,   474,  1433,  1389,   901,  1138,  1436,
     900,  1471,  1390,  1391,   953,  1566,   632,  1567,   954,  1392,
     638,   387,   388,  1427,   831,   380,  1113,  1114,  1115,   716,
     380,  1341,  1342,  1343,  1344,   414,  1195,  1196,  1197,   415,
    1489,  1490,  1139,  1529,  1530,  1531,  1581,  1079,  1582,   507,
     508,   561,   562,   576,   577,   726,   727,   498,   499,   420,
    1140,  1141,   421,   566,  1142,   930,   931,   422,   949,   940,
     941,   423,  1095,  1096,  1097,   424,   717,   938,   939,   963,
     964,   776,   777,   891,   718,   719,   720,   721,   722,  1052,
    1053,  1070,  1071,  1105,  1106,   425,  1008,  1223,  1345,  1346,
    1352,  1353,  1359,  1360,  1363,  1364,   426,  1503,   694,  1486,
     427,  1528,   428,  1446,  1447,  1453,  1454,  1458,  1459,  1507,
     429,   670,   671,   677,   678,  1057,   430,   723,   433,  1513,
    1514,   380,   679,   680,   396,   397,  1543,   724,  1602,  1603,
     687,   688,   914,   914,   434,   435,   442,   446,   447,   329,
     380,   896,   465,   462,  1511,   466,   467,   480,  1525,  1456,
     481,   497,  -222,   509,   511,   510,   512,   518,  1465,  1466,
     519,   316,   567,   532,   573,   401,   579,   588,   405,   410,
    1451,  1516,  1517,   416,   453,   621,   380,   639,   603,   590,
    -222,   619,  1548,   597,   598,   610,   622,   380,  -222,   630,
     642,   649,   830,   631,   650,   669,   830,   651,   681,   682,
     683,   684,   703,  1094,   704,   705,   706,   818,   836,   848,
     838,  1549,   849,   850,   851,  1562,  1554,   855,  1273,  1110,
     856,  1588,   859,   882,  1429,  1103,   884,   886,   885,   895,
    1432,   929,   887,   932,   380,   934,   936,  1007,  1008,   569,
    1561,  1063,  1441,  1019,   831,  1003,  1012,  1021,   831,  1032,
    1055,   380,  1074,  1064,  1168,   380,  1065,  1083,  1287,   380,
    1084,  1291,  1292,  1293,  1085,   380,   380,  1569,  1301,  1081,
    1086,   380,   379,  1098,  1099,  1090,   380,  1080,  1584,  1586,
     380,  1193,   891,  1117,  1104,  1119,  1534,  1535,  1122,  1161,
    1124,  1126,  1121,   566,   830,  1129,  1154,   566,  1130,  1152,
    1206,  1155,  1159,  1160,  1209,  1170,   830,  1338,  1339,  1167,
    1172,  1176,  1177,  1199,  1560,  1220,   830,   830,  1276,  1217,
    1608,  1611,  1614,  1617,  1157,  1158,  1222,   380,  1221,  1372,
    1247,  1277,  1619,  1227,  1228,  1278,  1286,  1288,  1235,  1294,
    1305,  1289,  1306,  1310,  1312,  1244,   831,  1314,  1319,  1318,
    1320,  1250,  1333,  1252,    98,  1370,  1255,  1275,   831,  1573,
    1259,  1373,  1374,   380,  1591,  1264,  1519,  1351,   831,   831,
    1522,  1377,  1269,  1378,  1384,  1385,  1271,  1386,  1387,   830,
    1402,  1418,  1422,  1419,  1423,  1430,  1431,  1438,  -222,  1439,
    1445,  -222,  -222,  -222,  -222,  1450,  -222,  -222,  1467,  -222,
    1448,  1452,  -222,  -222,  -222,  -222,  -222,  -222,  -222,  -222,
    -222,  1405,   830,  -222,  -222,  -222,  1468,  1476,  1482,  -222,
    -222,   713,   714,   715,   482,   483,   484,   485,   486,   487,
    1484,   831,  1414,  1502,  1515,   488,  1417,   380,  1521,  1556,
    1557,  -222,  -222,  1570,  -222,  1571,  1578,  1596,   489,  1605,
    1618,    15,   974,  -222,  1219,    14,  -222,  -222,  1348,  1006,
    1241,  1497,  1224,   707,   831,   380,  1481,  1061,   475,   865,
     868,  1108,  1311,   870,   445,  1354,   533,   380,  1437,   534,
    1298,  1193,   716,  1440,   555,   535,   380,  1455,   652,  1428,
     653,   702,  1128,  1382,  1518,   452,   915,  1062,   636,   606,
     595,  1093,  1193,   596,  1332,  1462,  1111,  1449,  1533,  1461,
    1123,  1321,  1356,  1538,  1545,   860,  1485,  1526,  1369,   490,
     491,   492,   493,   494,   495,     0,     0,     0,     0,   717,
       0,     0,     0,     0,     0,     0,     0,   718,   719,   720,
     721,   722,     0,  1403,     0,     0,     0,     0,  -317,  -317,
     757,     0,     0,   758,   759,   760,   761,     0,   762,     0,
       0,   763,     0,     0,   764,   765,   766,   767,   768,   769,
     770,   771,   772,     0,     0,   773,   774,   775,     0,     0,
     723,   496,     0,     0,     0,     0,   380,  -317,     0,     0,
     724,     0,   380,   380,     0,     0,     0,   380,     0,     0,
    -317,  -317,  -317,  -317,   380,     0,     0,     0,     0,  1520,
       0,     0,     0,  1523,     0,    11,  1524,     0,   776,   777,
       0,     0,     0,     0,   379,     0,     0,     0,     0,     0,
     379,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   379,  1207,     0,     0,   758,   759,   760,   761,
       0,   762,     0,     0,   763,  1546,     0,   764,   765,   766,
     767,   768,   769,   770,   771,   772,     0,     0,   773,   774,
     775,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   380,   380,     0,     0,   380,     0,   380,   380,   380,
     380,     0,     0,     0,     0,   380,     0,  1491,  1492,     0,
       0,  1193,     0,  1193,  1193,  1193,  1193,   380,    11,     0,
       0,   776,   777,     0,     0,     0,     0,   380,   380,  1504,
       0,     0,     0,     0,     0,     0,     0,     0,   380,     0,
       0,  1508,   380,     0,     0,     0,  1210,     0,     0,   758,
     759,   760,   761,     0,   762,     0,     0,   763,  1597,  1598,
     764,   765,   766,   767,   768,   769,   770,   771,   772,     0,
       0,   773,   774,   775,     0,     0,   379,     0,     0,     0,
     379,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     380,     0,   380,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1193,     0,
       0,    11,     0,     0,   776,   777,     0,     0,     0,     0,
       0,     0,     0,   380,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1558,   338,   339,   340,   341,   342,   343,
     344,   345,   346,    48,    49,    50,    51,    52,    53,    54,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,    67,    68,    69,    70,    71,    72,    73,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,   359,    87,     0,     0,     0,    88,    89,    90,
      91,    92,    93,    94,     0,     0,     0,     0,     0,   360,
     361,    97,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   362,   104,   363,   106,     0,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   364,   365,   118,   366,   120,
       0,   121,   367,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   368,   137,   138,   139,
     140,   141,   369,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,     0,   155,   156,   157,     0,
     370,   159,   371,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   372,   186,   187,
     373,   189,   190,   191,   192,   193,   374,   195,   375,   197,
     198,   199,   200,   201,   202,   203,   376,   377,   206,   207,
     208,   209,   210,   211,   212,   213,   214,     0,   758,   759,
     760,   761,     0,   762,     0,     0,   763,     0,     0,   764,
     765,   766,   767,   768,   769,   770,   771,   772,     0,     0,
     773,   774,   775,     0,     0,     0,     0,    22,     0,    23,
       0,     0,     0,     0,     0,     0,     0,     0,    24,     0,
       0,   692,    25,     0,     0,    26,    27,     0,    28,     0,
      29,     0,    30,    31,    32,     0,    33,    34,    35,   693,
      11,     0,     0,   776,   777,     0,     0,  1100,     0,  1101,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,     0,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,     0,     0,     0,    88,    89,    90,    91,
      92,    93,    94,     0,     0,     0,     0,     0,    95,    96,
      97,     0,     0,    98,    99,   100,   101,     0,     0,   102,
     103,   104,   105,   106,     0,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,     0,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,     0,   155,   156,   157,     0,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,    22,     0,    23,     0,
       0,     0,     0,     0,     0,     0,     0,    24,     0,     0,
     692,    25,     0,     0,    26,    27,     0,    28,     0,    29,
       0,    30,    31,    32,     0,    33,    34,    35,   701,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,     0,     0,     0,    88,    89,    90,    91,    92,
      93,    94,     0,     0,     0,     0,     0,    95,    96,    97,
       0,     0,    98,    99,   100,   101,     0,     0,   102,   103,
     104,   105,   106,     0,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,     0,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,     0,   155,   156,   157,     0,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,    22,     0,    23,     0,     0,
       0,     0,     0,     0,     0,     0,    24,     0,     0,     0,
      25,     0,     0,    26,    27,     0,    28,     0,    29,     0,
      30,    31,    32,     0,    33,    34,    35,   398,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,     0,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,     0,     0,     0,    88,    89,    90,    91,    92,    93,
      94,     0,     0,     0,     0,     0,    95,    96,    97,     0,
       0,    98,    99,   100,   101,     0,     0,   102,   103,   104,
     105,   106,     0,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,     0,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,     0,   155,   156,   157,     0,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,    22,     0,    23,     0,     0,     0,
       0,     0,     0,     0,     0,    24,     0,     0,     0,    25,
       0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
      31,    32,     0,    33,    34,    35,     0,     0,     0,     0,
       0,     0,     0,     0,   685,     0,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
       0,     0,     0,    88,    89,    90,    91,    92,    93,    94,
       0,     0,     0,     0,     0,    95,    96,    97,     0,     0,
      98,    99,   100,   101,     0,     0,   102,   103,   104,   105,
     106,     0,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,     0,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,     0,   155,   156,   157,     0,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,    22,     0,    23,     0,     0,     0,     0,
       0,     0,     0,     0,    24,     0,     0,     0,    25,     0,
       0,    26,    27,     0,    28,     0,    29,     0,    30,    31,
      32,     0,    33,    34,    35,     0,     0,     0,     0,     0,
       0,     0,     0,   845,     0,     0,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,     0,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,     0,
       0,     0,    88,    89,    90,    91,    92,    93,    94,     0,
       0,     0,     0,     0,    95,    96,    97,     0,     0,    98,
      99,   100,   101,     0,     0,   102,   103,   104,   105,   106,
       0,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,     0,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
       0,   155,   156,   157,     0,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,    22,     0,    23,     0,     0,     0,     0,     0,
       0,     0,     0,    24,     0,     0,     0,    25,     0,     0,
      26,    27,     0,    28,     0,    29,     0,    30,    31,    32,
       0,    33,    34,    35,     0,     0,     0,     0,     0,     0,
       0,     0,   853,     0,     0,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,     0,     0,
       0,    88,    89,    90,    91,    92,    93,    94,     0,     0,
       0,     0,     0,    95,    96,    97,     0,     0,    98,    99,
     100,   101,     0,     0,   102,   103,   104,   105,   106,     0,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,     0,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,     0,
     155,   156,   157,     0,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,    22,     0,    23,     0,     0,     0,     0,     0,     0,
       0,     0,    24,     0,     0,   692,    25,     0,     0,    26,
      27,     0,    28,     0,    29,     0,    30,    31,    32,     0,
      33,    34,    35,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,     0,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,     0,     0,     0,
      88,    89,    90,    91,    92,    93,    94,     0,     0,     0,
       0,     0,    95,    96,    97,     0,     0,    98,    99,   100,
     101,     0,     0,   102,   103,   104,   105,   106,     0,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,     0,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,     0,   155,
     156,   157,     0,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
      22,     0,    23,     0,     0,     0,     0,     0,     0,     0,
       0,    24,     0,     0,     0,    25,     0,     0,    26,    27,
       0,    28,     0,    29,     0,    30,    31,    32,     0,    33,
      34,    35,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,     0,     0,     0,    88,
      89,    90,    91,    92,    93,    94,     0,     0,     0,     0,
       0,    95,    96,    97,     0,     0,    98,    99,   100,   101,
       0,     0,   102,   103,   104,   105,   106,     0,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,     0,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,     0,   155,   156,
     157,     0,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,    22,
       0,    23,     0,     0,     0,     0,     0,     0,     0,     0,
      24,     0,     0,     0,    25,     0,     0,    26,    27,     0,
      28,     0,    29,     0,    30,    31,    32,     0,    33,    34,
      35,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
       0,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,   359,    87,     0,     0,     0,    88,    89,
      90,    91,    92,    93,    94,     0,     0,     0,     0,     0,
      95,    96,    97,     0,     0,    98,    99,   100,   101,     0,
       0,   102,   362,   104,   363,   106,     0,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   364,   365,   118,   366,
     120,     0,   121,   367,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,     0,   155,   156,   157,
       0,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   372,   186,
     187,   373,   189,   190,   191,   192,   193,   374,   195,   375,
     197,   198,   199,   200,   201,   202,   203,   376,   377,   206,
     207,   208,   209,   210,   211,   212,   213,   214,    22,     0,
      23,     0,     0,     0,     0,     0,     0,     0,     0,    24,
       0,     0,     0,    25,     0,     0,    26,    27,     0,    28,
       0,    29,     0,    30,    31,    32,     0,    33,    34,    35,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,     0,     0,     0,    88,    89,    90,
      91,    92,    93,    94,     0,     0,     0,     0,     0,    95,
      96,    97,     0,     0,    98,    99,   100,   101,     0,     0,
     102,   103,   104,   105,   106,     0,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
       0,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   536,
     150,   151,   152,   153,   154,     0,   155,   537,   157,     0,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,    22,     0,    23,
       0,     0,     0,     0,     0,     0,     0,     0,    24,     0,
       0,     0,    25,     0,     0,    26,    27,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    33,    34,    35,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,     0,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,   359,    87,     0,     0,     0,    88,    89,    90,    91,
      92,    93,    94,     0,     0,     0,     0,     0,    95,    96,
      97,     0,     0,    98,    99,   100,   101,     0,     0,   102,
     362,   104,   363,   106,     0,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   364,   365,   118,   366,   120,     0,
     121,   367,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   368,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,     0,   155,   156,   157,     0,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   372,   186,   187,   373,
     189,   190,   191,   192,   193,   374,   195,   375,   197,   198,
     199,   200,   201,   202,   203,   376,   377,   206,   207,   208,
     209,   210,   211,   212,   213,   214,    23,     0,     0,     0,
       0,     0,     0,     0,     0,  1179,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1180,     0,     0,     0,     0,
       0,  1181,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1182,   382,    41,    42,    43,   383,    45,    46,   384,    48,
      49,    50,    51,    52,    53,    54,  1183,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,    67,    68,
      69,    70,    71,    72,    73,     0,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,   359,    87,
       0,     0,     0,    88,    89,    90,    91,    92,    93,    94,
       0,     0,     0,     0,     0,   360,   361,    97,     0,     0,
       0,    99,   100,   101,     0,     0,   102,   362,   104,   363,
     106,     0,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   364,   365,   118,   366,   120,     0,   121,   367,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   368,   137,   138,   139,   140,   141,   369,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,     0,   155,   156,   157,     0,   385,   159,   371,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   372,   186,   187,   373,   189,   190,   191,
     192,   193,   374,   195,   375,   197,   198,  1184,  1185,   201,
     202,   203,   376,   377,   206,   207,   208,   209,   210,   211,
     212,   213,   214,    23,     0,     0,     0,     0,     0,     0,
       0,     0,  1179,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1182,   382,    41,
      42,    43,   383,    45,    46,   384,    48,    49,    50,    51,
      52,    53,    54,  1183,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,    67,    68,    69,    70,    71,
      72,    73,     0,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,   359,    87,     0,     0,     0,
      88,    89,    90,    91,    92,    93,    94,     0,     0,     0,
       0,     0,   360,   361,    97,     0,     0,     0,    99,   100,
     101,     0,     0,   102,   362,   104,   363,   106,     0,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   364,   365,
     118,   366,   120,     0,   121,   367,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   368,
     137,   138,   139,   140,   141,   369,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,     0,   155,
     156,   157,     0,   385,   159,   371,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     372,   186,   187,   373,   189,   190,   191,   192,   193,   374,
     195,   375,   197,   198,   199,   200,   201,   202,   203,   376,
     377,   206,   207,   208,   209,   210,   211,   212,   213,   214,
      23,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   381,   382,    41,    42,    43,   383,
      45,    46,   384,    48,    49,    50,    51,    52,    53,    54,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,    67,    68,    69,    70,    71,    72,    73,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,   359,    87,     0,     0,     0,    88,    89,    90,
      91,    92,    93,    94,     0,     0,     0,     0,     0,   360,
     361,    97,     0,     0,     0,    99,   100,   101,     0,     0,
     102,   362,   104,   363,   106,     0,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   364,   365,   118,   366,   120,
       0,   121,   367,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   368,   137,   138,   139,
     140,   141,   369,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,     0,   155,   156,   157,     0,
     385,   159,   371,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   372,   186,   187,
     373,   189,   190,   191,   192,   193,   374,   195,   375,   197,
     198,   199,   200,   201,   202,   203,   376,   377,   206,   207,
     208,   209,   210,   211,   212,   213,   214,  1279,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   823,  1280,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   381,   382,    41,    42,    43,   383,    45,    46,   384,
     824,    49,    50,    51,    52,    53,    54,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,    67,
      68,    69,    70,    71,    72,    73,     0,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,   359,
      87,     0,     0,     0,    88,    89,    90,    91,    92,    93,
      94,     0,     0,     0,     0,     0,   360,   361,    97,     0,
       0,     0,    99,     0,     0,     0,     0,   102,   362,   104,
     363,   106,     0,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   364,   365,   118,   366,   120,     0,   121,   367,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   368,   137,   138,   139,   140,   141,   369,
     825,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,     0,   155,   156,   157,     0,   385,   159,   826,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   372,   186,   187,   373,   189,   190,
     191,   192,   193,   374,   195,   375,   197,   198,   199,   200,
     201,   202,   203,   376,   377,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   582,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   583,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   338,   339,
     340,   341,   342,   343,   344,   345,   346,    48,    49,    50,
      51,    52,    53,    54,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,    67,    68,    69,    70,
      71,    72,    73,     0,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,   359,    87,     0,     0,
       0,    88,    89,    90,    91,    92,    93,    94,     0,     0,
       0,     0,     0,   360,   361,    97,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   362,   104,   363,   106,     0,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   364,
     365,   118,   366,   120,     0,   121,   367,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     368,   137,   138,   139,   140,   141,   369,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,     0,
     155,   156,   157,     0,   370,   159,   371,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   372,   186,   187,   373,   189,   190,   191,   192,   193,
     374,   195,   375,   197,   198,   199,   200,   201,   202,   203,
     376,   377,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   633,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   634,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   338,   339,   340,   341,   342,
     343,   344,   345,   346,    48,    49,    50,    51,    52,    53,
      54,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,    67,    68,    69,    70,    71,    72,    73,
       0,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,   359,    87,     0,     0,     0,    88,    89,
      90,    91,    92,    93,    94,     0,     0,     0,     0,     0,
     360,   361,    97,     0,     0,     0,    99,     0,     0,     0,
       0,   102,   362,   104,   363,   106,     0,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   364,   365,   118,   366,
     120,     0,   121,   367,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   368,   137,   138,
     139,   140,   141,   369,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,     0,   155,   156,   157,
       0,   370,   159,   371,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   372,   186,
     187,   373,   189,   190,   191,   192,   193,   374,   195,   375,
     197,   198,   199,   200,   201,   202,   203,   376,   377,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   823,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   381,   382,    41,    42,    43,   383,    45,
      46,   384,   824,    49,    50,    51,    52,    53,    54,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,    67,    68,    69,    70,    71,    72,    73,     0,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,   359,    87,     0,     0,     0,    88,    89,    90,    91,
      92,    93,    94,     0,     0,     0,     0,     0,   360,   361,
      97,     0,     0,     0,    99,     0,     0,     0,     0,   102,
     362,   104,   363,   106,     0,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   364,   365,   118,   366,   120,     0,
     121,   367,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   368,   137,   138,   139,   140,
     141,   369,   825,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,     0,   155,   156,   157,     0,   385,
     159,   826,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   372,   186,   187,   373,
     189,   190,   191,   192,   193,   374,   195,   375,   197,   198,
     199,   200,   201,   202,   203,   376,   377,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   823,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   381,   382,    41,    42,    43,   383,    45,    46,   384,
      48,    49,    50,    51,    52,    53,    54,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,    67,
      68,    69,    70,    71,    72,    73,     0,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,   359,
      87,     0,     0,     0,    88,    89,    90,    91,    92,    93,
      94,     0,     0,     0,     0,     0,   360,   361,    97,     0,
       0,     0,    99,     0,     0,     0,     0,   102,   362,   104,
     363,   106,     0,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   364,   365,   118,   366,   120,     0,   121,   367,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   368,   137,   138,   139,   140,   141,   369,
     825,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,     0,   155,   156,   157,     0,   385,   159,   826,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   372,   186,   187,   373,   189,   190,
     191,   192,   193,   374,   195,   375,   197,   198,   199,   200,
     201,   202,   203,   376,   377,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   599,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   338,   339,
     340,   341,   342,   343,   344,   345,   346,    48,    49,    50,
      51,    52,    53,    54,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,    67,    68,    69,    70,
      71,    72,    73,     0,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,   359,    87,     0,     0,
       0,    88,    89,    90,    91,    92,    93,    94,     0,     0,
       0,     0,     0,   360,   361,    97,     0,     0,   600,    99,
       0,     0,     0,     0,   102,   362,   104,   363,   106,     0,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   364,
     365,   118,   366,   120,     0,   121,   367,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     368,   137,   138,   139,   140,   141,   369,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,     0,
     155,   156,   157,     0,   370,   159,   371,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   372,   186,   187,   373,   189,   190,   191,   192,   193,
     374,   195,   375,   197,   198,   199,   200,   201,   202,   203,
     376,   377,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   338,   339,   340,   341,   342,   343,   344,   345,   346,
      48,    49,    50,    51,    52,    53,    54,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,    67,
      68,    69,    70,    71,    72,    73,     0,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,   359,
      87,     0,     0,     0,    88,    89,    90,    91,    92,    93,
      94,     0,     0,     0,     0,     0,   360,   361,    97,     0,
       0,     0,    99,     0,     0,     0,     0,   102,   362,   104,
     363,   106,     0,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   364,   365,   118,   366,   120,     0,   121,   367,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   368,   137,   138,   139,   140,   141,   369,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,     0,   155,   156,   157,     0,   370,   159,   371,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   372,   186,   187,   373,   189,   190,
     191,   192,   193,   374,   195,   375,   197,   198,   199,   200,
     201,   202,   203,   376,   377,   206,   207,   208,   209,   210,
     211,   212,   213,   214,    49,    50,    51,    52,    53,    54,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,    67,    68,    69,    70,    71,    72,    73,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,   359,    87,     0,     0,     0,    88,    89,    90,
      91,    92,    93,    94,     0,     0,     0,     0,     0,   360,
     361,    97,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   362,   104,   363,   106,     0,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   364,   365,   118,     0,   120,
       0,   121,     0,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   368,   137,   138,   139,
     140,   141,   369,     0,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,     0,   155,   156,   157,     0,
       0,   159,     0,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   372,   186,   187,
     373,   189,   190,   191,   192,   193,   374,   195,   375,   197,
     198,   199,   200,   201,   202,   203,   376,   377,   206,   207,
     208,   209,   210,   211,   212,   213,   214,     0,  1230,     0,
       0,   758,   759,   760,   761,     0,   762,     0,     0,   763,
       0,     0,   764,   765,   766,   767,   768,   769,   770,   771,
     772,     0,     0,   773,   774,   775,  1242,     0,     0,   758,
     759,   760,   761,     0,   762,     0,     0,   763,     0,     0,
     764,   765,   766,   767,   768,   769,   770,   771,   772,     0,
       0,   773,   774,   775,  1251,     0,     0,   758,   759,   760,
     761,     0,   762,    11,     0,   763,   776,   777,   764,   765,
     766,   767,   768,   769,   770,   771,   772,     0,     0,   773,
     774,   775,  1253,     0,     0,   758,   759,   760,   761,     0,
     762,    11,     0,   763,   776,   777,   764,   765,   766,   767,
     768,   769,   770,   771,   772,     0,     0,   773,   774,   775,
    1256,     0,     0,   758,   759,   760,   761,     0,   762,    11,
       0,   763,   776,   777,   764,   765,   766,   767,   768,   769,
     770,   771,   772,     0,     0,   773,   774,   775,  1257,     0,
       0,   758,   759,   760,   761,     0,   762,    11,     0,   763,
     776,   777,   764,   765,   766,   767,   768,   769,   770,   771,
     772,     0,     0,   773,   774,   775,  1262,     0,     0,   758,
     759,   760,   761,     0,   762,    11,     0,   763,   776,   777,
     764,   765,   766,   767,   768,   769,   770,   771,   772,     0,
       0,   773,   774,   775,  1265,     0,     0,   758,   759,   760,
     761,     0,   762,    11,     0,   763,   776,   777,   764,   765,
     766,   767,   768,   769,   770,   771,   772,     0,     0,   773,
     774,   775,  1268,     0,     0,   758,   759,   760,   761,     0,
     762,    11,     0,   763,   776,   777,   764,   765,   766,   767,
     768,   769,   770,   771,   772,     0,     0,   773,   774,   775,
    1270,     0,     0,   758,   759,   760,   761,     0,   762,    11,
       0,   763,   776,   777,   764,   765,   766,   767,   768,   769,
     770,   771,   772,     0,     0,   773,   774,   775,  1272,     0,
       0,   758,   759,   760,   761,     0,   762,    11,     0,   763,
     776,   777,   764,   765,   766,   767,   768,   769,   770,   771,
     772,     0,     0,   773,   774,   775,  1400,     0,     0,   758,
     759,   760,   761,     0,   762,    11,     0,   763,   776,   777,
     764,   765,   766,   767,   768,   769,   770,   771,   772,     0,
       0,   773,   774,   775,  1401,     0,     0,   758,   759,   760,
     761,     0,   762,    11,     0,   763,   776,   777,   764,   765,
     766,   767,   768,   769,   770,   771,   772,     0,     0,   773,
     774,   775,  1406,     0,     0,   758,   759,   760,   761,     0,
     762,    11,     0,   763,   776,   777,   764,   765,   766,   767,
     768,   769,   770,   771,   772,     0,     0,   773,   774,   775,
    1412,     0,     0,   758,   759,   760,   761,     0,   762,    11,
       0,   763,   776,   777,   764,   765,   766,   767,   768,   769,
     770,   771,   772,     0,     0,   773,   774,   775,  1505,     0,
       0,   758,   759,   760,   761,     0,   762,    11,     0,   763,
     776,   777,   764,   765,   766,   767,   768,   769,   770,   771,
     772,     0,     0,   773,   774,   775,  1506,     0,     0,   758,
     759,   760,   761,     0,   762,    11,     0,   763,   776,   777,
     764,   765,   766,   767,   768,   769,   770,   771,   772,     0,
       0,   773,   774,   775,  1509,     0,     0,   758,   759,   760,
     761,     0,   762,    11,     0,   763,   776,   777,   764,   765,
     766,   767,   768,   769,   770,   771,   772,     0,     0,   773,
     774,   775,  1510,     0,     0,   758,   759,   760,   761,     0,
     762,    11,     0,   763,   776,   777,   764,   765,   766,   767,
     768,   769,   770,   771,   772,   985,     0,   773,   774,   775,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
       0,     0,   776,   777,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   986,     0,   987,   988,   989,
     990,   991,   992,   993,     0,     0,     0,    11,     0,     0,
     776,   777,   994,   995,   996,   997,   998,   999,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1000,  1001,  1002
};

static const yytype_int16 yycheck[] =
{
      22,   160,    24,   500,   575,   618,   243,   620,   962,     6,
     623,  1099,    63,   669,   531,   410,   669,  1167,    24,   639,
    1142,   538,   539,   540,  1327,   542,   543,   544,  1347,   122,
     401,   501,   502,   503,   504,   552,    35,   554,    22,    28,
     557,   897,    40,   414,   415,     9,   430,    40,   640,  1371,
      48,   435,   122,    33,   438,    59,    59,  1385,   442,   726,
     727,   681,   682,    11,   122,  1273,    12,    32,    10,    41,
     749,   122,   751,    34,    35,    40,    12,    32,  1188,    48,
       9,    40,   453,    48,    49,    40,     9,    52,    53,    54,
      62,  1487,    48,    48,    34,    35,     9,    52,    53,    54,
      50,   119,   781,   152,    34,    35,   720,    45,    40,    40,
      48,   725,   791,    47,    48,    19,    48,   796,   119,   798,
      40,   800,    70,   802,    40,   804,   101,   806,    48,   808,
      33,   248,   105,   812,    59,   814,    40,   532,   225,    52,
      53,    54,    24,   125,    48,    27,    28,    32,   280,    31,
     573,   818,    12,   240,   296,   114,   298,   130,    33,   115,
     125,   280,    45,    48,  1372,    48,    48,     9,  1471,   144,
     125,   114,    19,   222,   223,   307,   293,   115,    21,   104,
      40,   230,   116,   114,   303,   569,   303,   205,    48,  1585,
     707,   173,   124,   211,   212,   131,   125,    40,   114,    41,
     161,    48,   573,   190,   124,   819,   175,   820,   131,  1417,
     211,   212,   125,     0,   162,   163,   639,  1171,   164,   191,
     124,   161,   226,   226,   183,   318,   176,   320,  1550,  1557,
     194,   253,   115,   175,   256,   117,   118,   621,   231,   219,
     183,   897,   240,   899,   897,   148,   899,   253,   318,   319,
     256,   244,   245,   246,   247,  1574,  1112,   246,   681,   682,
     318,   319,   929,  1385,  1386,   649,   249,   183,   639,   125,
     321,   322,   323,   148,  1577,     9,   943,   124,   243,  1389,
    1390,  1391,  1392,   213,   214,   215,   150,    19,   243,    68,
      69,   970,   959,    33,   279,   974,    47,    48,    40,     3,
       4,     5,     6,     7,     8,   327,    48,    41,    40,    49,
     681,   682,   683,   684,   280,    23,   458,   173,   303,   183,
     462,    95,   717,   102,   466,   467,   468,   183,   325,   471,
     313,    48,  1011,    41,  1484,    30,   302,   303,   271,    32,
     114,    36,   138,   327,  1023,   280,   120,    40,   968,   145,
    1472,    68,    69,   973,   248,    48,   250,  1445,   978,    52,
      53,    54,   982,   257,   258,   116,    11,   180,   303,   263,
     303,   306,   307,   395,    25,   308,   309,   303,    23,   401,
     306,   523,   276,   277,   526,    33,   192,   193,   410,    34,
      40,    42,   414,   415,    33,   191,   192,   193,    48,    33,
     823,    49,    44,   402,    46,    33,   280,   406,   430,  1029,
      49,   395,   411,   435,   280,    49,   438,  1539,   417,   173,
     442,    49,   196,   197,   198,   231,   410,   201,   202,   303,
     429,   453,   125,   278,   433,   434,   302,   303,   244,   245,
     246,   247,   287,   465,    33,  1065,   430,   446,   447,   448,
     280,   435,   823,    33,   438,   454,  1112,  1579,   442,  1112,
      49,    33,   292,  1119,    33,  1121,  1119,    40,  1121,    49,
     129,   842,   302,   615,    33,   280,    33,    49,    51,    33,
      49,   465,    41,   625,   626,   290,   291,  1154,    33,   248,
    1157,  1158,    49,   279,   280,    49,   255,   302,   303,  1070,
    1071,  1014,   248,   180,    49,  1172,   176,   878,   650,   255,
     532,   188,  1025,  1026,   513,   301,   302,   187,   304,   518,
     519,  1034,  1201,   522,    22,  1038,    33,  1040,  1041,    33,
    1043,    33,    22,  1046,  1047,    33,  1490,    41,    33,    41,
    1153,  1058,    49,    33,  1223,   968,   107,   569,   532,    32,
     973,   573,   936,   280,    49,   978,   698,   699,   700,   982,
    1239,   280,   957,   290,   291,    48,   961,    22,  1247,  1161,
     567,   290,  1251,    33,    33,   302,   303,    33,    33,    33,
      22,    41,   105,   302,   303,   569,    33,    41,    33,   588,
      49,    33,    33,    49,    41,   220,    41,   968,    19,   621,
      41,    33,   973,   228,   603,    33,  1029,   978,    33,    41,
      40,   982,    85,    41,  1131,    88,    41,   639,    91,    92,
      93,    94,    33,   171,    40,    33,    33,   649,    33,   177,
      41,   630,   631,    41,    41,    19,    41,   621,    65,    66,
      67,    33,  1065,   482,   483,   484,   485,   486,   487,    41,
     489,   490,   491,   492,   493,   494,   495,   496,  1029,   681,
     682,   683,   684,   900,   186,   649,   131,   132,   190,   134,
     135,   136,    40,   138,   139,  1295,    30,  1333,   151,  1299,
    1333,  1348,    36,    30,   236,    44,   452,    46,   240,    36,
     456,   122,   123,  1285,  1065,   717,   208,   209,   210,   126,
     722,    91,    92,    93,    94,    40,    52,    53,    54,    40,
    1377,  1378,   185,   213,   214,   215,   164,   859,   166,   165,
     166,   314,   315,   105,   106,   189,   190,    34,    35,    19,
     203,   204,    19,   717,   207,   171,   172,    19,   722,   120,
     121,    19,   884,   885,   886,    19,   173,   171,   172,   288,
     289,   316,   317,   895,   181,   182,   183,   184,   185,   293,
     294,   122,   123,   140,   141,    19,   260,   261,    86,    87,
      86,    87,   143,   144,   178,   179,    19,  1397,   920,  1371,
      19,  1448,    19,   143,   144,    99,   100,   216,   217,  1409,
      48,   498,   499,   505,   506,   817,     9,   224,    48,  1419,
    1420,   823,   507,   508,    30,    31,  1473,   234,  1581,  1582,
     516,   517,   683,   684,    48,     9,     9,    40,    40,    40,
     842,   660,   234,    59,  1416,   239,    33,   156,  1441,  1326,
     157,   195,    12,   158,   160,   169,   168,    21,  1335,  1336,
      21,   190,   180,   173,   152,    40,    29,    48,    40,    40,
    1320,  1422,  1423,    40,    40,     9,   878,   152,    48,    41,
      40,    33,  1482,    41,    41,    41,    33,   889,    48,    48,
     159,     9,  1295,    48,   137,    63,  1299,   138,   159,   152,
     152,   152,    52,   882,   125,   125,    50,    37,   105,    41,
     106,  1483,    41,    41,    41,  1515,  1488,    41,  1057,   898,
      41,  1557,    10,    48,  1288,   889,   225,   152,    59,   137,
    1294,    37,   225,    84,   936,   125,     9,   303,   260,     9,
    1512,    40,  1306,   280,  1295,   248,   305,   248,  1299,   303,
     303,   953,   130,    40,   956,   957,    40,    10,  1080,   961,
      10,  1083,  1084,  1085,   133,   967,   968,  1518,  1090,   114,
     150,   973,   936,    40,    40,   154,   978,   131,  1550,  1551,
     982,   967,  1104,    90,    33,   241,  1463,  1464,    83,   953,
     199,    23,   242,   957,  1397,   230,    37,   961,   230,    84,
     979,   142,    33,   125,   983,   191,  1409,  1129,  1130,   235,
     114,   289,   288,   257,  1511,   248,  1419,  1420,    41,   122,
    1592,  1593,  1594,  1595,   173,   173,   305,  1029,  1007,  1168,
     272,    41,  1604,  1012,  1013,    41,    41,     9,  1017,     9,
     225,   194,     9,   225,   142,  1024,  1397,    41,   213,    97,
      96,  1030,   133,  1032,   125,    33,  1035,  1059,  1409,  1536,
    1039,    37,   173,  1065,  1561,  1044,  1430,   213,  1419,  1420,
    1434,    33,  1051,    37,    19,    40,  1055,    40,    12,  1482,
     122,    41,    43,   152,    43,     9,   133,   155,   248,    41,
      40,   251,   252,   253,   254,    49,   256,   257,   200,   259,
     145,    96,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   248,  1515,   273,   274,   275,   213,   125,   152,   279,
     280,    65,    66,    67,    13,    14,    15,    16,    17,    18,
     235,  1482,   248,    21,   152,    24,  1275,  1139,   152,    41,
      33,   301,   302,   131,   304,    10,   206,    26,    37,    41,
     206,     4,   747,   313,  1002,     3,   316,   317,   114,   759,
    1020,  1387,  1009,   530,  1515,  1167,  1366,   821,   219,   619,
     622,   895,  1104,   624,   117,  1139,   318,  1179,  1300,   318,
    1087,  1167,   126,  1305,   322,   321,  1188,  1325,   480,  1285,
     481,   526,   920,  1179,  1424,   142,   684,   823,   453,   414,
     407,   878,  1188,   407,  1121,  1333,   899,  1315,  1457,  1329,
     908,  1119,  1152,  1470,  1475,   614,  1370,  1442,  1161,   108,
     109,   110,   111,   112,   113,    -1,    -1,    -1,    -1,   173,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,   182,   183,
     184,   185,    -1,  1222,    -1,    -1,    -1,    -1,   192,   193,
     248,    -1,    -1,   251,   252,   253,   254,    -1,   256,    -1,
      -1,   259,    -1,    -1,   262,   263,   264,   265,   266,   267,
     268,   269,   270,    -1,    -1,   273,   274,   275,    -1,    -1,
     224,   170,    -1,    -1,    -1,    -1,  1288,   231,    -1,    -1,
     234,    -1,  1294,  1295,    -1,    -1,    -1,  1299,    -1,    -1,
     244,   245,   246,   247,  1306,    -1,    -1,    -1,    -1,  1431,
      -1,    -1,    -1,  1435,    -1,   313,  1438,    -1,   316,   317,
      -1,    -1,    -1,    -1,  1288,    -1,    -1,    -1,    -1,    -1,
    1294,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1306,   248,    -1,    -1,   251,   252,   253,   254,
      -1,   256,    -1,    -1,   259,  1477,    -1,   262,   263,   264,
     265,   266,   267,   268,   269,   270,    -1,    -1,   273,   274,
     275,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1383,  1384,    -1,    -1,  1387,    -1,  1389,  1390,  1391,
    1392,    -1,    -1,    -1,    -1,  1397,    -1,  1383,  1384,    -1,
      -1,  1387,    -1,  1389,  1390,  1391,  1392,  1409,   313,    -1,
      -1,   316,   317,    -1,    -1,    -1,    -1,  1419,  1420,  1398,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1430,    -1,
      -1,  1410,  1434,    -1,    -1,    -1,   248,    -1,    -1,   251,
     252,   253,   254,    -1,   256,    -1,    -1,   259,  1570,  1571,
     262,   263,   264,   265,   266,   267,   268,   269,   270,    -1,
      -1,   273,   274,   275,    -1,    -1,  1430,    -1,    -1,    -1,
    1434,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1482,    -1,  1484,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1484,    -1,
      -1,   313,    -1,    -1,   316,   317,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1515,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1502,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    -1,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,   114,    -1,    -1,    -1,    -1,    -1,   120,
     121,   122,    -1,    -1,    -1,   126,    -1,    -1,    -1,    -1,
     131,   132,   133,   134,   135,    -1,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
      -1,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,    -1,   187,   188,   189,    -1,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,    -1,   251,   252,
     253,   254,    -1,   256,    -1,    -1,   259,    -1,    -1,   262,
     263,   264,   265,   266,   267,   268,   269,   270,    -1,    -1,
     273,   274,   275,    -1,    -1,    -1,    -1,     9,    -1,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    -1,
      -1,    23,    24,    -1,    -1,    27,    28,    -1,    30,    -1,
      32,    -1,    34,    35,    36,    -1,    38,    39,    40,    41,
     313,    -1,    -1,   316,   317,    -1,    -1,   318,    -1,   320,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    -1,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,    -1,    -1,    -1,   108,   109,   110,   111,
     112,   113,   114,    -1,    -1,    -1,    -1,    -1,   120,   121,
     122,    -1,    -1,   125,   126,   127,   128,    -1,    -1,   131,
     132,   133,   134,   135,    -1,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,    -1,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,    -1,   187,   188,   189,    -1,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,     9,    -1,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    -1,    -1,
      23,    24,    -1,    -1,    27,    28,    -1,    30,    -1,    32,
      -1,    34,    35,    36,    -1,    38,    39,    40,    41,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    -1,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,    -1,    -1,    -1,   108,   109,   110,   111,   112,
     113,   114,    -1,    -1,    -1,    -1,    -1,   120,   121,   122,
      -1,    -1,   125,   126,   127,   128,    -1,    -1,   131,   132,
     133,   134,   135,    -1,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,    -1,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,    -1,   187,   188,   189,    -1,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,     9,    -1,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    20,    -1,    -1,    -1,
      24,    -1,    -1,    27,    28,    -1,    30,    -1,    32,    -1,
      34,    35,    36,    -1,    38,    39,    40,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,   112,   113,
     114,    -1,    -1,    -1,    -1,    -1,   120,   121,   122,    -1,
      -1,   125,   126,   127,   128,    -1,    -1,   131,   132,   133,
     134,   135,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,    -1,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,    -1,   187,   188,   189,    -1,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,     9,    -1,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    20,    -1,    -1,    -1,    24,
      -1,    -1,    27,    28,    -1,    30,    -1,    32,    -1,    34,
      35,    36,    -1,    38,    39,    40,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    49,    -1,    -1,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    -1,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
      -1,    -1,    -1,   108,   109,   110,   111,   112,   113,   114,
      -1,    -1,    -1,    -1,    -1,   120,   121,   122,    -1,    -1,
     125,   126,   127,   128,    -1,    -1,   131,   132,   133,   134,
     135,    -1,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,    -1,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,    -1,   187,   188,   189,    -1,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,     9,    -1,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    20,    -1,    -1,    -1,    24,    -1,
      -1,    27,    28,    -1,    30,    -1,    32,    -1,    34,    35,
      36,    -1,    38,    39,    40,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    -1,    -1,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    -1,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,    -1,
      -1,    -1,   108,   109,   110,   111,   112,   113,   114,    -1,
      -1,    -1,    -1,    -1,   120,   121,   122,    -1,    -1,   125,
     126,   127,   128,    -1,    -1,   131,   132,   133,   134,   135,
      -1,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,    -1,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
      -1,   187,   188,   189,    -1,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,     9,    -1,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    20,    -1,    -1,    -1,    24,    -1,    -1,
      27,    28,    -1,    30,    -1,    32,    -1,    34,    35,    36,
      -1,    38,    39,    40,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    -1,    -1,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    -1,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,   112,   113,   114,    -1,    -1,
      -1,    -1,    -1,   120,   121,   122,    -1,    -1,   125,   126,
     127,   128,    -1,    -1,   131,   132,   133,   134,   135,    -1,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,    -1,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,    -1,
     187,   188,   189,    -1,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,     9,    -1,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    20,    -1,    -1,    23,    24,    -1,    -1,    27,
      28,    -1,    30,    -1,    32,    -1,    34,    35,    36,    -1,
      38,    39,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    -1,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,   112,   113,   114,    -1,    -1,    -1,
      -1,    -1,   120,   121,   122,    -1,    -1,   125,   126,   127,
     128,    -1,    -1,   131,   132,   133,   134,   135,    -1,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,    -1,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,    -1,   187,
     188,   189,    -1,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
       9,    -1,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    20,    -1,    -1,    -1,    24,    -1,    -1,    27,    28,
      -1,    30,    -1,    32,    -1,    34,    35,    36,    -1,    38,
      39,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    -1,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,    -1,    -1,    -1,   108,
     109,   110,   111,   112,   113,   114,    -1,    -1,    -1,    -1,
      -1,   120,   121,   122,    -1,    -1,   125,   126,   127,   128,
      -1,    -1,   131,   132,   133,   134,   135,    -1,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,    -1,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,    -1,   187,   188,
     189,    -1,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,     9,
      -1,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      20,    -1,    -1,    -1,    24,    -1,    -1,    27,    28,    -1,
      30,    -1,    32,    -1,    34,    35,    36,    -1,    38,    39,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      -1,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,   114,    -1,    -1,    -1,    -1,    -1,
     120,   121,   122,    -1,    -1,   125,   126,   127,   128,    -1,
      -1,   131,   132,   133,   134,   135,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,    -1,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,    -1,   187,   188,   189,
      -1,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     9,    -1,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,
      -1,    -1,    -1,    24,    -1,    -1,    27,    28,    -1,    30,
      -1,    32,    -1,    34,    35,    36,    -1,    38,    39,    40,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    -1,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,   114,    -1,    -1,    -1,    -1,    -1,   120,
     121,   122,    -1,    -1,   125,   126,   127,   128,    -1,    -1,
     131,   132,   133,   134,   135,    -1,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
      -1,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,    -1,   187,   188,   189,    -1,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,     9,    -1,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    -1,
      -1,    -1,    24,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    38,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    -1,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,    -1,    -1,    -1,   108,   109,   110,   111,
     112,   113,   114,    -1,    -1,    -1,    -1,    -1,   120,   121,
     122,    -1,    -1,   125,   126,   127,   128,    -1,    -1,   131,
     132,   133,   134,   135,    -1,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,    -1,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,    -1,   187,   188,   189,    -1,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    20,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    -1,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
      -1,    -1,    -1,   108,   109,   110,   111,   112,   113,   114,
      -1,    -1,    -1,    -1,    -1,   120,   121,   122,    -1,    -1,
      -1,   126,   127,   128,    -1,    -1,   131,   132,   133,   134,
     135,    -1,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,    -1,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,    -1,   187,   188,   189,    -1,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    -1,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,   112,   113,   114,    -1,    -1,    -1,
      -1,    -1,   120,   121,   122,    -1,    -1,    -1,   126,   127,
     128,    -1,    -1,   131,   132,   133,   134,   135,    -1,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,    -1,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,    -1,   187,
     188,   189,    -1,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    -1,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,   114,    -1,    -1,    -1,    -1,    -1,   120,
     121,   122,    -1,    -1,    -1,   126,   127,   128,    -1,    -1,
     131,   132,   133,   134,   135,    -1,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
      -1,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,    -1,   187,   188,   189,    -1,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    40,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,   112,   113,
     114,    -1,    -1,    -1,    -1,    -1,   120,   121,   122,    -1,
      -1,    -1,   126,    -1,    -1,    -1,    -1,   131,   132,   133,
     134,   135,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,    -1,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,    -1,   187,   188,   189,    -1,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    -1,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,   112,   113,   114,    -1,    -1,
      -1,    -1,    -1,   120,   121,   122,    -1,    -1,    -1,   126,
      -1,    -1,    -1,    -1,   131,   132,   133,   134,   135,    -1,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,    -1,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,    -1,
     187,   188,   189,    -1,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      -1,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,   114,    -1,    -1,    -1,    -1,    -1,
     120,   121,   122,    -1,    -1,    -1,   126,    -1,    -1,    -1,
      -1,   131,   132,   133,   134,   135,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,    -1,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,    -1,   187,   188,   189,
      -1,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    -1,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,    -1,    -1,    -1,   108,   109,   110,   111,
     112,   113,   114,    -1,    -1,    -1,    -1,    -1,   120,   121,
     122,    -1,    -1,    -1,   126,    -1,    -1,    -1,    -1,   131,
     132,   133,   134,   135,    -1,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,    -1,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,    -1,   187,   188,   189,    -1,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,   112,   113,
     114,    -1,    -1,    -1,    -1,    -1,   120,   121,   122,    -1,
      -1,    -1,   126,    -1,    -1,    -1,    -1,   131,   132,   133,
     134,   135,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,    -1,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,    -1,   187,   188,   189,    -1,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,    41,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    -1,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,   112,   113,   114,    -1,    -1,
      -1,    -1,    -1,   120,   121,   122,    -1,    -1,   125,   126,
      -1,    -1,    -1,    -1,   131,   132,   133,   134,   135,    -1,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,    -1,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,    -1,
     187,   188,   189,    -1,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,   112,   113,
     114,    -1,    -1,    -1,    -1,    -1,   120,   121,   122,    -1,
      -1,    -1,   126,    -1,    -1,    -1,    -1,   131,   132,   133,
     134,   135,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,    -1,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,    -1,   187,   188,   189,    -1,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    -1,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,   114,    -1,    -1,    -1,    -1,    -1,   120,
     121,   122,    -1,    -1,    -1,   126,    -1,    -1,    -1,    -1,
     131,   132,   133,   134,   135,    -1,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,    -1,   150,
      -1,   152,    -1,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,    -1,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,    -1,   187,   188,   189,    -1,
      -1,   192,    -1,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,    -1,   248,    -1,
      -1,   251,   252,   253,   254,    -1,   256,    -1,    -1,   259,
      -1,    -1,   262,   263,   264,   265,   266,   267,   268,   269,
     270,    -1,    -1,   273,   274,   275,   248,    -1,    -1,   251,
     252,   253,   254,    -1,   256,    -1,    -1,   259,    -1,    -1,
     262,   263,   264,   265,   266,   267,   268,   269,   270,    -1,
      -1,   273,   274,   275,   248,    -1,    -1,   251,   252,   253,
     254,    -1,   256,   313,    -1,   259,   316,   317,   262,   263,
     264,   265,   266,   267,   268,   269,   270,    -1,    -1,   273,
     274,   275,   248,    -1,    -1,   251,   252,   253,   254,    -1,
     256,   313,    -1,   259,   316,   317,   262,   263,   264,   265,
     266,   267,   268,   269,   270,    -1,    -1,   273,   274,   275,
     248,    -1,    -1,   251,   252,   253,   254,    -1,   256,   313,
      -1,   259,   316,   317,   262,   263,   264,   265,   266,   267,
     268,   269,   270,    -1,    -1,   273,   274,   275,   248,    -1,
      -1,   251,   252,   253,   254,    -1,   256,   313,    -1,   259,
     316,   317,   262,   263,   264,   265,   266,   267,   268,   269,
     270,    -1,    -1,   273,   274,   275,   248,    -1,    -1,   251,
     252,   253,   254,    -1,   256,   313,    -1,   259,   316,   317,
     262,   263,   264,   265,   266,   267,   268,   269,   270,    -1,
      -1,   273,   274,   275,   248,    -1,    -1,   251,   252,   253,
     254,    -1,   256,   313,    -1,   259,   316,   317,   262,   263,
     264,   265,   266,   267,   268,   269,   270,    -1,    -1,   273,
     274,   275,   248,    -1,    -1,   251,   252,   253,   254,    -1,
     256,   313,    -1,   259,   316,   317,   262,   263,   264,   265,
     266,   267,   268,   269,   270,    -1,    -1,   273,   274,   275,
     248,    -1,    -1,   251,   252,   253,   254,    -1,   256,   313,
      -1,   259,   316,   317,   262,   263,   264,   265,   266,   267,
     268,   269,   270,    -1,    -1,   273,   274,   275,   248,    -1,
      -1,   251,   252,   253,   254,    -1,   256,   313,    -1,   259,
     316,   317,   262,   263,   264,   265,   266,   267,   268,   269,
     270,    -1,    -1,   273,   274,   275,   248,    -1,    -1,   251,
     252,   253,   254,    -1,   256,   313,    -1,   259,   316,   317,
     262,   263,   264,   265,   266,   267,   268,   269,   270,    -1,
      -1,   273,   274,   275,   248,    -1,    -1,   251,   252,   253,
     254,    -1,   256,   313,    -1,   259,   316,   317,   262,   263,
     264,   265,   266,   267,   268,   269,   270,    -1,    -1,   273,
     274,   275,   248,    -1,    -1,   251,   252,   253,   254,    -1,
     256,   313,    -1,   259,   316,   317,   262,   263,   264,   265,
     266,   267,   268,   269,   270,    -1,    -1,   273,   274,   275,
     248,    -1,    -1,   251,   252,   253,   254,    -1,   256,   313,
      -1,   259,   316,   317,   262,   263,   264,   265,   266,   267,
     268,   269,   270,    -1,    -1,   273,   274,   275,   248,    -1,
      -1,   251,   252,   253,   254,    -1,   256,   313,    -1,   259,
     316,   317,   262,   263,   264,   265,   266,   267,   268,   269,
     270,    -1,    -1,   273,   274,   275,   248,    -1,    -1,   251,
     252,   253,   254,    -1,   256,   313,    -1,   259,   316,   317,
     262,   263,   264,   265,   266,   267,   268,   269,   270,    -1,
      -1,   273,   274,   275,   248,    -1,    -1,   251,   252,   253,
     254,    -1,   256,   313,    -1,   259,   316,   317,   262,   263,
     264,   265,   266,   267,   268,   269,   270,    -1,    -1,   273,
     274,   275,   248,    -1,    -1,   251,   252,   253,   254,    -1,
     256,   313,    -1,   259,   316,   317,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   248,    -1,   273,   274,   275,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   313,
      -1,    -1,   316,   317,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   278,    -1,   280,   281,   282,
     283,   284,   285,   286,    -1,    -1,    -1,   313,    -1,    -1,
     316,   317,   295,   296,   297,   298,   299,   300,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   310,   311,   312
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,   325,   326,   327,
     249,   313,   335,   336,   358,   327,   190,   331,   332,   330,
     453,     0,     9,    11,    20,    24,    27,    28,    30,    32,
      34,    35,    36,    38,    39,    40,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   108,   109,
     110,   111,   112,   113,   114,   120,   121,   122,   125,   126,
     127,   128,   131,   132,   133,   134,   135,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   187,   188,   189,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   460,   461,   462,   463,   464,
     465,   467,   472,   483,   486,   492,   493,   494,   495,   497,
     498,   499,   500,   501,   502,   503,   504,   505,   506,   507,
     508,   509,   510,   511,   512,   514,   515,   516,   517,   518,
     519,   520,   521,   522,   523,   524,   525,   526,   527,   528,
     529,   530,   531,   532,   533,   534,   536,   537,   538,   539,
     540,   543,   544,   545,   556,   558,   560,   561,   562,   565,
     567,   568,   569,   570,   580,   581,   582,   583,   584,   585,
     586,   587,   589,   591,   593,   634,   635,   636,   637,   638,
     639,   640,   643,   644,   645,   646,   648,   655,   656,   660,
     661,   662,   663,   669,   670,   186,   190,   419,   420,   423,
     424,   425,   426,   337,   359,   180,   328,   173,   333,    40,
     231,   244,   245,   246,   247,   451,   454,   455,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,   103,
     120,   121,   132,   134,   146,   147,   149,   153,   167,   173,
     191,   193,   218,   221,   227,   229,   237,   238,   535,   669,
     670,    55,    56,    60,    63,   191,   527,   122,   123,   546,
     129,   107,   557,   516,   105,   548,   507,   507,    41,   461,
      19,    40,    48,   124,   566,    40,    48,    40,    40,    40,
      40,    48,   124,   571,    40,    40,    40,    48,    19,    19,
      19,    19,    19,    19,    19,    19,    19,    19,    19,    48,
       9,   641,   642,    48,    48,     9,   468,   469,     9,   194,
     473,   474,     9,   484,   485,   484,    40,    40,    48,    68,
      69,    19,   571,    40,    48,   124,   563,   451,    59,   104,
      59,   226,    59,    59,   226,   234,   239,    33,   131,   132,
     134,   135,   136,   138,   139,   465,   466,   475,   476,   477,
     156,   157,    13,    14,    15,    16,    17,    18,    24,    37,
     108,   109,   110,   111,   112,   113,   170,   195,    34,    35,
     161,    11,    70,   162,   163,    12,   164,   165,   166,   158,
     169,   160,   168,    48,   512,   516,    30,    36,    21,    21,
     527,   527,    21,    40,   462,   462,    40,    51,   176,   187,
     421,   422,   173,   423,   424,   426,   180,   188,   427,   428,
     430,   431,   432,   433,   434,   435,   436,   437,   440,   441,
     442,   445,   446,   448,   449,   460,   633,   649,   278,   287,
     338,   314,   315,   361,   453,   647,   669,   180,   334,     9,
      41,   457,   458,   152,   456,   547,   105,   106,   559,    29,
     669,    41,    11,    41,   588,   595,   648,   461,    48,   564,
      41,   461,    41,    62,   191,   591,   593,    41,    41,    41,
     125,   647,   461,    48,   572,   590,   595,   594,   596,   648,
      41,   461,   461,   535,    33,   219,   461,   461,   535,    33,
     535,     9,    33,   535,    33,   148,   148,   461,   461,   461,
      48,    48,   564,    11,    41,   592,   596,   461,   564,   152,
     575,   462,   159,   462,   669,   462,   462,   462,   462,     9,
     137,   138,   494,   495,   497,   497,   497,   497,   497,   497,
     496,   497,   497,   497,   497,   497,   497,   497,   497,    63,
     500,   500,   499,   501,   501,   501,   501,   502,   502,   503,
     503,   159,   152,   152,   152,    49,   461,   517,   517,   461,
     461,   461,    23,    41,   462,   541,   542,   152,   222,   223,
     230,    41,   541,    52,   125,   125,    50,   422,   429,   429,
     647,   180,   188,    65,    66,    67,   126,   173,   181,   182,
     183,   184,   185,   224,   234,   453,   189,   190,   429,   429,
     429,   429,   429,   429,   429,   429,   429,   248,   250,   257,
     258,   263,   276,   277,   339,   340,   343,   344,   348,   349,
     350,   351,   352,   353,   356,   357,   360,   248,   251,   252,
     253,   254,   256,   259,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   273,   274,   275,   316,   317,   358,   362,
     363,   364,   365,   366,   367,   368,   371,   372,   376,   377,
     378,   379,   380,   385,   386,   389,   390,   391,   392,   393,
     394,   395,   396,   397,   398,   399,   400,   401,   402,   403,
     404,   405,   406,   407,   408,   192,   193,   450,    37,   453,
     535,    33,    41,    40,    64,   174,   193,   576,   578,   579,
     580,   648,   664,   665,   666,   668,   105,   548,   106,   105,
     130,   513,    33,    41,    49,    49,   461,    49,    41,    41,
      41,    41,    49,    49,   461,    41,    41,    49,    49,    10,
     642,   462,    49,    49,   575,   469,   575,   535,   474,   575,
     485,   462,   462,    41,    41,    49,   461,   461,    33,    41,
      49,   576,    48,   459,   225,    59,   152,   225,   225,   240,
     535,   462,   478,   479,   480,   137,   497,    40,    48,   243,
     511,   532,   599,   602,   603,   604,   605,   606,   607,   608,
     609,   576,   576,   573,   579,   573,    49,    22,    22,    22,
      33,    41,   220,   228,   462,   462,   462,    41,   429,    37,
     171,   172,    84,   616,   125,   598,     9,   647,   171,   172,
     120,   121,   138,   145,   191,   450,   171,   177,   438,   669,
      68,    69,   102,   236,   240,   652,   450,   173,   183,   444,
     598,   173,   598,   288,   289,   279,   280,   301,   302,   304,
     345,   280,   292,   302,   345,   280,   290,   291,   302,   303,
     361,   280,   302,   303,   361,   248,   278,   280,   281,   282,
     283,   284,   285,   286,   295,   296,   297,   298,   299,   300,
     310,   311,   312,   248,   115,   459,   366,   303,   260,   381,
     382,   383,   305,   303,   306,   361,   279,   303,   370,   280,
     373,   248,   361,   271,   303,   308,   309,   387,   280,   302,
     303,   361,   303,   361,   280,   303,   361,   361,   280,   303,
     306,   307,   361,   280,   303,   361,   280,   307,   361,   248,
     293,   303,   293,   294,   361,   303,   361,   670,   598,   450,
     575,   458,   578,    40,    40,    40,    11,    23,    34,   577,
     122,   123,    25,    42,   130,   597,   648,    49,    49,   462,
     131,   114,   470,    10,    10,   133,   150,   487,   489,   490,
     154,    49,    49,   597,   461,   462,   462,   462,    40,    40,
     318,   320,   657,   669,    33,   140,   141,   481,   478,   599,
     461,   606,    48,   208,   209,   210,   610,    90,   632,   241,
     601,   242,    83,   616,   199,   611,    23,   574,   542,   230,
     230,   598,    85,    88,    91,    92,    93,    94,   151,   185,
     203,   204,   207,   617,   618,   619,   620,   621,   624,   629,
     630,   631,    84,   535,    37,   142,   598,   173,   173,    33,
     125,   669,   122,   318,   319,   653,   654,   235,   670,   647,
     191,   598,   114,   443,   647,   443,   289,   288,   341,    20,
      30,    36,    55,    71,   232,   233,   409,   410,   411,   412,
     415,   416,   417,   527,   576,    52,    53,    54,   342,   257,
     346,   347,   361,   576,   361,   576,   461,   248,   576,   461,
     248,    63,   122,   321,   322,   323,   354,   122,   355,   355,
     248,   461,   305,   261,   382,   384,   361,   461,   461,   360,
     248,   122,   318,   320,   369,   461,   248,   255,   374,   375,
     248,   374,   248,   361,   461,   360,   360,   272,   388,   576,
     461,   248,   461,   248,   360,   461,   248,   248,   360,   461,
     360,   360,   248,   360,   461,   248,   360,   360,   248,   461,
     248,   461,   248,   451,   429,   670,    41,    41,    41,    11,
      41,   576,   667,   548,   548,   555,    41,   462,     9,   194,
     471,   462,   462,   462,     9,   491,   183,   488,   489,    12,
     131,   462,    23,    41,    49,   225,     9,   658,   659,   658,
     225,   479,   142,   482,    41,    49,    49,   599,    97,   213,
      96,   603,    95,   114,   120,   196,   197,   198,   201,   202,
     600,   613,   604,   133,   119,   211,   212,   612,   462,   462,
     429,    91,    92,    93,    94,    86,    87,    40,   114,   183,
     623,   213,    86,    87,   669,   646,   617,   575,   598,   143,
     144,   598,   598,   178,   179,   439,    40,   459,   651,   652,
      33,   409,   451,    37,   173,   443,   598,    33,    37,   122,
     318,   319,   527,    19,    19,    40,    40,    12,   416,    30,
      36,    30,    36,   418,   280,   290,   291,   302,   303,   361,
     248,   248,   122,   461,   361,   248,   248,   280,   290,   302,
     303,   361,   248,   361,   248,   361,   456,   451,    41,   152,
      33,    41,    43,    43,    31,   117,   118,   459,   544,   535,
       9,   133,   535,   576,     9,   131,   576,   462,   155,    41,
     462,   535,    33,    41,    41,    40,   143,   144,   145,   610,
      49,   501,    96,    99,   100,   498,   499,   612,   216,   217,
     615,   615,   605,   101,   144,   499,   499,   200,   213,   183,
     623,   598,    40,   114,   183,   626,   125,    10,   175,   447,
      41,   457,   152,   650,   235,   654,   459,   651,   456,   598,
     598,   527,   527,   413,   534,   646,   646,   410,   416,   416,
     416,   416,    21,   576,   461,   248,   248,   576,   461,   248,
     248,   459,   456,   576,   576,   152,   548,   548,   546,   535,
     462,   152,   535,   462,   462,   575,   659,   658,   598,   213,
     214,   215,   614,   614,   499,   499,   161,   622,   622,   205,
     612,   627,   646,   598,   625,   625,   462,    41,   576,   459,
     409,   650,   175,   452,   459,   443,    41,    33,   461,   329,
     429,   459,   576,    44,    46,   549,    44,    46,   550,   548,
     131,    10,    41,   499,    33,    41,    41,   646,   206,    33,
      41,   164,   166,   628,   459,   651,   459,   414,   532,   534,
      22,   429,   551,   553,   552,   554,    26,   462,   462,   623,
     612,   646,   626,   626,   650,    41,    45,   115,   459,    47,
     116,   459,    45,   115,   459,    47,   116,   459,   206,   459
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, &yylloc, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, &yylloc)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  /* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;
/* Location data for the look-ahead symbol.  */
YYLTYPE yylloc;

  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;

  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[2];

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;
#if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 0;
#endif

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
	YYSTACK_RELOCATE (yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 8:
#line 775 "../src/parser/XQParser.y"
    {
    SET_BUILT_IN_NAMESPACE(XQillaFunction::XQillaPrefix, XQillaFunction::XMLChFunctionURI);
  }
    break;

  case 9:
#line 782 "../src/parser/XQParser.y"
    {
    SET_BUILT_IN_NAMESPACE(XMLChXS, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    SET_BUILT_IN_NAMESPACE(XMLChXSI, SchemaSymbols::fgURI_XSI);
    SET_BUILT_IN_NAMESPACE(XMLChFN, XQFunction::XMLChFunctionURI);
    SET_BUILT_IN_NAMESPACE(XMLChLOCAL, XQUserFunction::XMLChXQueryLocalFunctionsURI);
    SET_BUILT_IN_NAMESPACE(XMLChERR, FunctionError::XMLChXQueryErrorURI);
    SET_BUILT_IN_NAMESPACE(XQillaFunction::XQillaPrefix, XQillaFunction::XMLChFunctionURI);
  }
    break;

  case 10:
#line 794 "../src/parser/XQParser.y"
    {
    (yyvsp[(2) - (8)].signature)->argSpecs = (yyvsp[(5) - (8)].argSpecs);
    (yyvsp[(2) - (8)].signature)->returnType = (yyvsp[(6) - (8)].sequenceType);
    QP->_function = WRAP((yylsp[(1) - (8)]), new (MEMMGR) XQUserFunction((yyvsp[(4) - (8)].str), (yyvsp[(2) - (8)].signature), (yyvsp[(7) - (8)].astNode), true, MEMMGR));
  }
    break;

  case 13:
#line 805 "../src/parser/XQParser.y"
    {
    (yyvsp[(1) - (3)].signature)->argSpecs = (yyvsp[(2) - (3)].argSpecs);
    (yyvsp[(1) - (3)].signature)->returnType = (yyvsp[(3) - (3)].sequenceType);
    QP->_signature = (yyvsp[(1) - (3)].signature);
  }
    break;

  case 14:
#line 814 "../src/parser/XQParser.y"
    {
    printf("  { 0, 0, 0, 0, 0 }\n};\n\n");

    UTF8Str module(QP->_moduleName);
    printf("static const DelayedModule %s_module = { %s_file, %s_prefix, %s_uri, %s_functions };\n",
           module.str(), module.str(), module.str(), module.str(), module.str());
  }
    break;

  case 15:
#line 825 "../src/parser/XQParser.y"
    {
    QP->_moduleName = (yyvsp[(3) - (6)].str);

    UTF8Str module(QP->_moduleName);
    UTF8Str file(QP->_lexer->getFile());

    printf("// Auto-generated by compile-delayed-module from %s\n", file.str());
    printf("// DO NOT EDIT\n\n");

    printf("// %s\n", file.str());
    printf("static const XMLCh %s_file[] = { ", module.str());
    const XMLCh *ptr = QP->_lexer->getFile();
    for(; *ptr != 0; ++ptr) {
      printf("%d, ", *ptr);
    }
    printf("0 };\n\n");

    printf("// %s\n", UTF8((yyvsp[(3) - (6)].str)));
    printf("static const XMLCh %s_prefix[] = { ", module.str());
    ptr = (yyvsp[(3) - (6)].str);
    for(; *ptr != 0; ++ptr) {
      printf("%d, ", *ptr);
    }
    printf("0 };\n\n");

    printf("// %s\n", UTF8((yyvsp[(5) - (6)].str)));
    printf("static const XMLCh %s_uri[] = { ", module.str());
    ptr = (yyvsp[(5) - (6)].str);
    for(; *ptr != 0; ++ptr) {
      printf("%d, ", *ptr);
    }
    printf("0 };\n\n");
  }
    break;

  case 16:
#line 862 "../src/parser/XQParser.y"
    {
    printf("static const DelayedModule::FuncDef %s_functions[] = {\n", UTF8(QP->_moduleName));
  }
    break;

  case 17:
#line 866 "../src/parser/XQParser.y"
    {
  }
    break;

  case 18:
#line 872 "../src/parser/XQParser.y"
    {
    XMLBuffer buf;

    const XMLCh *localname = XPath2NSUtils::getLocalName((yyvsp[(4) - (8)].str));

    printf("  {\n    \"%s\", %d, %s, %d, %d,\n", UTF8(localname), (int)((yyvsp[(5) - (8)].argSpecs) ? (yyvsp[(5) - (8)].argSpecs)->size() : 0),
           (yyvsp[(2) - (8)].signature)->privateOption == FunctionSignature::OP_TRUE ? "true" : "false", (yylsp[(1) - (8)]).first_line, (yylsp[(1) - (8)]).first_column);
    printf("    \"");
    const XMLCh *ptr = ((XQLexer*)QP->_lexer)->getQueryString() + (yylsp[(1) - (8)]).first_offset;
    const XMLCh *start = ptr;
    const XMLCh *end = ((XQLexer*)QP->_lexer)->getQueryString() + (yylsp[(8) - (8)]).last_offset;
    for(;ptr < end; ++ptr) {
      if(*ptr == '"') {
        if((ptr - start) == 0) buf.reset();
        else buf.set(start, ptr - start);
        printf("%s\\\"", UTF8(buf.getRawBuffer()));
        start = ptr + 1;
      }
      else if(*ptr == '\n') {
        if((ptr - start) == 0) buf.reset();
        else buf.set(start, ptr - start);
        printf("%s\\n\"\n    \"", UTF8(buf.getRawBuffer()));
        start = ptr + 1;
      }
    }
    buf.set(start, ptr - start);
    printf("%s\\n\"\n", UTF8(buf.getRawBuffer()));
    printf("  },\n");
  }
    break;

  case 19:
#line 908 "../src/parser/XQParser.y"
    {
    SequenceType *optionalString =
      WRAP((yylsp[(1) - (1)]), new (MEMMGR) SequenceType(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                         AnyAtomicType::fgDT_ANYATOMICTYPE,
                                         SequenceType::QUESTION_MARK, MEMMGR));

    XQGlobalVariable *nameVar =
      WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQGlobalVariable(0, optionalString,
                                             WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQSequence(MEMMGR)), MEMMGR, /*isParam*/true));
    nameVar->setVariableURI(XQillaFunction::XMLChFunctionURI);
    nameVar->setVariableLocalName(var_name);

    QP->_query->addVariable(nameVar);

    ASTNode *nameVarRef1 = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQVariable(XQillaFunction::XMLChFunctionURI, var_name, MEMMGR));
    XQCallTemplate *call = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQCallTemplate(nameVarRef1, 0, MEMMGR));

    ASTNode *ci = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQContextItem(MEMMGR));
    ASTNode *apply = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQApplyTemplates(ci, 0, 0, MEMMGR));

    VectorOfASTNodes *args = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    args->push_back(WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQVariable(XQillaFunction::XMLChFunctionURI, var_name, MEMMGR)));
    ASTNode *exists = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQFunctionCall(0, XQFunction::XMLChFunctionURI,
                                                           MEMMGR->getPooledString("exists"), args, MEMMGR));

    QP->_query->setQueryBody(WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQIf(exists, call, apply, MEMMGR)));
  }
    break;

  case 20:
#line 939 "../src/parser/XQParser.y"
    {
  }
    break;

  case 21:
#line 942 "../src/parser/XQParser.y"
    {
    // TBD Check for xsl:version attr - jpcs
    NodeTest *nt = new (MEMMGR) NodeTest();
    nt->setNodeType(Node::document_string);
    nt->setNameWildcard();
    nt->setNamespaceWildcard();

    VectorOfASTNodes *pattern = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    pattern->push_back(WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQStep(XQStep::SELF, nt, MEMMGR)));

    XQUserFunction::ModeList *modelist = new (MEMMGR) XQUserFunction::ModeList(XQillaAllocator<XQUserFunction::Mode*>(MEMMGR));
    modelist->push_back(WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::DEFAULT)));

    FunctionSignature *signature = new (MEMMGR) FunctionSignature(MEMMGR);
    XQUserFunction *func = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQUserFunction(0, pattern, signature, (yyvsp[(1) - (1)].astNode), MEMMGR));
    func->setModeList(modelist);

    QP->_query->addFunction(func);
  }
    break;

  case 23:
#line 966 "../src/parser/XQParser.y"
    {
    // TBD Check the value - jpcs
  }
    break;

  case 24:
#line 970 "../src/parser/XQParser.y"
    {
    // TBD implement exclude-result-prefixes - jpcs
  }
    break;

  case 32:
#line 988 "../src/parser/XQParser.y"
    {
    if((yyvsp[(1) - (4)].functDecl)->getName() == 0 && (yyvsp[(1) - (4)].functDecl)->getPattern() == 0) {
      yyerror((yylsp[(1) - (4)]), "The xsl:template declaration does not have either a {}name or {}match attribute, or both [err:XTSE0500]");
    }

    if((yyvsp[(1) - (4)].functDecl)->getPattern() != 0 && (yyvsp[(1) - (4)].functDecl)->getModeList() == 0) {
      XQUserFunction::ModeList *modelist = new (MEMMGR) XQUserFunction::ModeList(XQillaAllocator<XQUserFunction::Mode*>(MEMMGR));
      modelist->push_back(WRAP((yylsp[(1) - (4)]), new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::DEFAULT)));
      (yyvsp[(1) - (4)].functDecl)->setModeList(modelist);
    }

    (yyvsp[(1) - (4)].functDecl)->getSignature()->argSpecs = (yyvsp[(2) - (4)].argSpecs);
    (yyvsp[(1) - (4)].functDecl)->setFunctionBody((yyvsp[(3) - (4)].parenExpr));
    QP->_query->addFunction((yyvsp[(1) - (4)].functDecl));
  }
    break;

  case 33:
#line 1007 "../src/parser/XQParser.y"
    {
    FunctionSignature *signature = new (MEMMGR) FunctionSignature(MEMMGR);
    (yyval.functDecl) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQUserFunction(0, 0, signature, 0, MEMMGR));
  }
    break;

  case 34:
#line 1012 "../src/parser/XQParser.y"
    {
    (yyval.functDecl) = (yyvsp[(1) - (3)].functDecl);
    (yyval.functDecl)->setPattern((yyvsp[(3) - (3)].itemList));
  }
    break;

  case 35:
#line 1017 "../src/parser/XQParser.y"
    {
    (yyval.functDecl) = (yyvsp[(1) - (2)].functDecl);
    RESOLVE_QNAME((yylsp[(2) - (2)]), (yyvsp[(2) - (2)].str));
    (yyval.functDecl)->setURI(uri);
    (yyval.functDecl)->setName(name);
  }
    break;

  case 36:
#line 1024 "../src/parser/XQParser.y"
    {
    (yyval.functDecl) = (yyvsp[(1) - (3)].functDecl);
    // TBD priority - jpcs
  }
    break;

  case 37:
#line 1029 "../src/parser/XQParser.y"
    {
    (yyval.functDecl) = (yyvsp[(1) - (3)].functDecl);
    (yyval.functDecl)->setModeList((yyvsp[(3) - (3)].modeList));
  }
    break;

  case 38:
#line 1034 "../src/parser/XQParser.y"
    {
    (yyval.functDecl) = (yyvsp[(1) - (3)].functDecl);
    (yyval.functDecl)->getSignature()->returnType = (yyvsp[(3) - (3)].sequenceType);
  }
    break;

  case 39:
#line 1042 "../src/parser/XQParser.y"
    {
    (yyval.modeList) = new (MEMMGR) XQUserFunction::ModeList(XQillaAllocator<XQUserFunction::Mode*>(MEMMGR));
  }
    break;

  case 40:
#line 1046 "../src/parser/XQParser.y"
    {
    RESOLVE_QNAME((yylsp[(2) - (2)]), (yyvsp[(2) - (2)].str));
    (yyvsp[(1) - (2)].modeList)->push_back(WRAP((yylsp[(2) - (2)]), new (MEMMGR) XQUserFunction::Mode(uri, name)));
    (yyval.modeList) = (yyvsp[(1) - (2)].modeList);
  }
    break;

  case 41:
#line 1052 "../src/parser/XQParser.y"
    {
    (yyvsp[(1) - (2)].modeList)->push_back(WRAP((yylsp[(2) - (2)]), new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::DEFAULT)));
    (yyval.modeList) = (yyvsp[(1) - (2)].modeList);
  }
    break;

  case 42:
#line 1057 "../src/parser/XQParser.y"
    {
    (yyvsp[(1) - (2)].modeList)->push_back(WRAP((yylsp[(2) - (2)]), new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::ALL)));
    (yyval.modeList) = (yyvsp[(1) - (2)].modeList);
  }
    break;

  case 46:
#line 1067 "../src/parser/XQParser.y"
    {
    if((yyvsp[(1) - (4)].functDecl)->getName() == 0) {
      yyerror((yylsp[(1) - (4)]), "The xsl:function declaration does not have a {}name attribute");
    }

    (yyvsp[(1) - (4)].functDecl)->getSignature()->argSpecs = (yyvsp[(2) - (4)].argSpecs);
    (yyvsp[(1) - (4)].functDecl)->setFunctionBody((yyvsp[(3) - (4)].parenExpr));
    QP->_query->addFunction((yyvsp[(1) - (4)].functDecl));
  }
    break;

  case 47:
#line 1080 "../src/parser/XQParser.y"
    {    
    FunctionSignature *signature = new (MEMMGR) FunctionSignature(MEMMGR);
    (yyval.functDecl) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQUserFunction(0, signature, 0, true, MEMMGR));
  }
    break;

  case 48:
#line 1085 "../src/parser/XQParser.y"
    {
    (yyval.functDecl) = (yyvsp[(1) - (2)].functDecl);
    RESOLVE_QNAME((yylsp[(2) - (2)]), (yyvsp[(2) - (2)].str));
    (yyval.functDecl)->setURI(uri);
    (yyval.functDecl)->setName(name);
  }
    break;

  case 49:
#line 1092 "../src/parser/XQParser.y"
    {
    (yyval.functDecl) = (yyvsp[(1) - (3)].functDecl);
    (yyval.functDecl)->getSignature()->returnType = (yyvsp[(3) - (3)].sequenceType);
  }
    break;

  case 50:
#line 1097 "../src/parser/XQParser.y"
    {
    (yyval.functDecl) = (yyvsp[(1) - (2)].functDecl);
    // TBD override - jpcs
  }
    break;

  case 51:
#line 1105 "../src/parser/XQParser.y"
    {
    (yyval.argSpecs) = new (MEMMGR) ArgumentSpecs(XQillaAllocator<ArgumentSpec*>(MEMMGR));
  }
    break;

  case 52:
#line 1109 "../src/parser/XQParser.y"
    {
    (yyval.argSpecs) = (yyvsp[(1) - (2)].argSpecs);
    (yyval.argSpecs)->push_back((yyvsp[(2) - (2)].argSpec));
  }
    break;

  case 53:
#line 1117 "../src/parser/XQParser.y"
    {
    (yyval.argSpec) = (yyvsp[(1) - (3)].argSpec);

    if((yyval.argSpec)->getName() == 0) {
      yyerror((yylsp[(1) - (3)]), "The xsl:param instruction does not have a {}name attribute");
    }

    if((yyval.argSpec)->getType() == 0) {
      (yyval.argSpec)->setType(WRAP((yylsp[(1) - (3)]), new (MEMMGR) SequenceType(new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ANYTHING), SequenceType::STAR)));
    }

    // TBD default parameter values - jpcs

/*     if(!$2->getChildren().empty()) { */
/*       if($$->value != 0) { */
/*         yyerror(@1, "The xsl:with-param instruction has both a select attribute and a sequence constructor [err:XTSE0870]"); */
/*       } */
/*       $$->value = $2; */
/*     } */
/*     else if($$->value == 0) { */
/*       yyerror(@1, "The xsl:with-param instruction has neither a select attribute nor a sequence constructor [err:XTSE0870]"); */
/*     } */
  }
    break;

  case 54:
#line 1144 "../src/parser/XQParser.y"
    {
    (yyval.argSpec) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) ArgumentSpec(0, 0, MEMMGR));
  }
    break;

  case 55:
#line 1148 "../src/parser/XQParser.y"
    {
    (yyval.argSpec) = (yyvsp[(1) - (2)].argSpec);
    RESOLVE_QNAME((yylsp[(2) - (2)]), (yyvsp[(2) - (2)].str));
    (yyval.argSpec)->setURI(uri);
    (yyval.argSpec)->setName(name);
  }
    break;

  case 56:
#line 1155 "../src/parser/XQParser.y"
    {
    (yyval.argSpec) = (yyvsp[(1) - (3)].argSpec);
    // TBD default parameter values - jpcs
/*     $$->value = PRESERVE_NS(@2, $3); */
  }
    break;

  case 57:
#line 1161 "../src/parser/XQParser.y"
    {
    (yyval.argSpec) = (yyvsp[(1) - (3)].argSpec);
    (yyval.argSpec)->setType((yyvsp[(3) - (3)].sequenceType));
  }
    break;

  case 58:
#line 1166 "../src/parser/XQParser.y"
    {
    (yyval.argSpec) = (yyvsp[(1) - (2)].argSpec);
    // TBD required - jpcs
  }
    break;

  case 59:
#line 1171 "../src/parser/XQParser.y"
    {
    (yyval.argSpec) = (yyvsp[(1) - (2)].argSpec);
    // TBD tunnel parameters - jpcs
  }
    break;

  case 60:
#line 1180 "../src/parser/XQParser.y"
    {
    if((yyvsp[(1) - (3)].globalVar)->isRequired()) {
      if((yyvsp[(1) - (3)].globalVar)->getVariableExpr() != 0 || !(yyvsp[(2) - (3)].parenExpr)->getChildren().empty()) {
        yyerror((yylsp[(1) - (3)]), "A required xsl:param declaration must be empty and must not specify a {}select attribute");
      }
    }
    else {
      (yyvsp[(1) - (3)].globalVar)->setVariableExpr(XSLT_VARIABLE_VALUE((yylsp[(1) - (3)]), (ASTNode*)(yyvsp[(1) - (3)].globalVar)->getVariableExpr(), (yyvsp[(2) - (3)].parenExpr), (SequenceType*)(yyvsp[(1) - (3)].globalVar)->getSequenceType()));

      if((yyvsp[(1) - (3)].globalVar)->getVariableExpr() == 0) {
        yyerror((yylsp[(1) - (3)]), "The xsl:param declaration has both a select attribute and a sequence constructor [err:XTSE0620]");
      }
    }

    if((yyvsp[(1) - (3)].globalVar)->getVariableLocalName() == 0) {
      yyerror((yylsp[(1) - (3)]), "The xsl:param declaration does not have a {}name attribute");
    }

    QP->_query->addVariable((yyvsp[(1) - (3)].globalVar));
  }
    break;

  case 61:
#line 1204 "../src/parser/XQParser.y"
    {
    (yyval.globalVar) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQGlobalVariable(0, 0, 0, MEMMGR, /*isParam*/true));
  }
    break;

  case 62:
#line 1208 "../src/parser/XQParser.y"
    {
    RESOLVE_QNAME((yylsp[(2) - (2)]), (yyvsp[(2) - (2)].str));
    (yyvsp[(1) - (2)].globalVar)->setVariableURI(uri);
    (yyvsp[(1) - (2)].globalVar)->setVariableLocalName(name);
    (yyval.globalVar) = (yyvsp[(1) - (2)].globalVar);
  }
    break;

  case 63:
#line 1215 "../src/parser/XQParser.y"
    {
    (yyvsp[(1) - (3)].globalVar)->setVariableExpr(PRESERVE_NS((yylsp[(2) - (3)]), (yyvsp[(3) - (3)].astNode)));
    (yyval.globalVar) = (yyvsp[(1) - (3)].globalVar);
  }
    break;

  case 64:
#line 1220 "../src/parser/XQParser.y"
    {
    (yyvsp[(1) - (3)].globalVar)->setSequenceType((yyvsp[(3) - (3)].sequenceType));
    (yyval.globalVar) = (yyvsp[(1) - (3)].globalVar);
  }
    break;

  case 65:
#line 1225 "../src/parser/XQParser.y"
    {
    (yyval.globalVar) = (yyvsp[(1) - (2)].globalVar);
    (yyval.globalVar)->setRequired((yyvsp[(2) - (2)].boolean));
  }
    break;

  case 66:
#line 1230 "../src/parser/XQParser.y"
    {
    (yyval.globalVar) = (yyvsp[(1) - (2)].globalVar);
    if((yyvsp[(2) - (2)].boolean)) {
      yyerror((yylsp[(2) - (2)]), "An xsl:param declaration cannot have a {}tunnel attribute with a value of \"yes\"");
    }
  }
    break;

  case 67:
#line 1241 "../src/parser/XQParser.y"
    {
    (yyvsp[(1) - (3)].globalVar)->setVariableExpr(XSLT_VARIABLE_VALUE((yylsp[(1) - (3)]), (ASTNode*)(yyvsp[(1) - (3)].globalVar)->getVariableExpr(), (yyvsp[(2) - (3)].parenExpr), (SequenceType*)(yyvsp[(1) - (3)].globalVar)->getSequenceType()));
    (yyvsp[(1) - (3)].globalVar)->setSequenceType(0);

    if((yyvsp[(1) - (3)].globalVar)->getVariableExpr() == 0) {
      yyerror((yylsp[(1) - (3)]), "The xsl:variable declaration has both a select attribute and a sequence constructor [err:XTSE0620]");
    }

    if((yyvsp[(1) - (3)].globalVar)->getVariableLocalName() == 0) {
      yyerror((yylsp[(1) - (3)]), "The xsl:variable declaration does not have a {}name attribute");
    }

    QP->_query->addVariable((yyvsp[(1) - (3)].globalVar));
  }
    break;

  case 68:
#line 1259 "../src/parser/XQParser.y"
    {
    (yyval.globalVar) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQGlobalVariable(0, 0, 0, MEMMGR));
  }
    break;

  case 69:
#line 1263 "../src/parser/XQParser.y"
    {
    RESOLVE_QNAME((yylsp[(2) - (2)]), (yyvsp[(2) - (2)].str));
    (yyvsp[(1) - (2)].globalVar)->setVariableURI(uri);
    (yyvsp[(1) - (2)].globalVar)->setVariableLocalName(name);
    (yyval.globalVar) = (yyvsp[(1) - (2)].globalVar);
  }
    break;

  case 70:
#line 1270 "../src/parser/XQParser.y"
    {
    (yyvsp[(1) - (3)].globalVar)->setVariableExpr(PRESERVE_NS((yylsp[(2) - (3)]), (yyvsp[(3) - (3)].astNode)));
    (yyval.globalVar) = (yyvsp[(1) - (3)].globalVar);
  }
    break;

  case 71:
#line 1275 "../src/parser/XQParser.y"
    {
    (yyvsp[(1) - (3)].globalVar)->setSequenceType((yyvsp[(3) - (3)].sequenceType));
    (yyval.globalVar) = (yyvsp[(1) - (3)].globalVar);
  }
    break;

  case 72:
#line 1283 "../src/parser/XQParser.y"
    {
    // TBD Add the output to the static context - jpcs
  }
    break;

  case 73:
#line 1308 "../src/parser/XQParser.y"
    {
  }
    break;

  case 74:
#line 1311 "../src/parser/XQParser.y"
    {
  }
    break;

  case 75:
#line 1314 "../src/parser/XQParser.y"
    {
  }
    break;

  case 76:
#line 1317 "../src/parser/XQParser.y"
    {
  }
    break;

  case 77:
#line 1320 "../src/parser/XQParser.y"
    {
  }
    break;

  case 78:
#line 1323 "../src/parser/XQParser.y"
    {
  }
    break;

  case 79:
#line 1326 "../src/parser/XQParser.y"
    {
  }
    break;

  case 80:
#line 1329 "../src/parser/XQParser.y"
    {
  }
    break;

  case 81:
#line 1332 "../src/parser/XQParser.y"
    {
  }
    break;

  case 82:
#line 1335 "../src/parser/XQParser.y"
    {
  }
    break;

  case 83:
#line 1338 "../src/parser/XQParser.y"
    {
  }
    break;

  case 84:
#line 1341 "../src/parser/XQParser.y"
    {
  }
    break;

  case 85:
#line 1344 "../src/parser/XQParser.y"
    {
  }
    break;

  case 86:
#line 1347 "../src/parser/XQParser.y"
    {
  }
    break;

  case 87:
#line 1350 "../src/parser/XQParser.y"
    {
  }
    break;

  case 88:
#line 1353 "../src/parser/XQParser.y"
    {
  }
    break;

  case 89:
#line 1356 "../src/parser/XQParser.y"
    {
  }
    break;

  case 90:
#line 1359 "../src/parser/XQParser.y"
    {
  }
    break;

  case 95:
#line 1369 "../src/parser/XQParser.y"
    {
//     $$ = $1;
    const XMLCh *p = (yyvsp[(1) - (1)].str);
    while(*p && *p != ':') ++p;
    if(*p == 0) {
      yyerror((yylsp[(1) - (1)]), "The method for the xsl:output declaration does not have a prefix");
    }
  }
    break;

  case 96:
#line 1381 "../src/parser/XQParser.y"
    {
  }
    break;

  case 97:
#line 1384 "../src/parser/XQParser.y"
    {
  }
    break;

  case 99:
#line 1392 "../src/parser/XQParser.y"
    {
    LOCATION((yylsp[(1) - (1)]), loc);
    CONTEXT->addSchemaLocation(XMLUni::fgZeroLenString, 0, &loc);
  }
    break;

  case 100:
#line 1397 "../src/parser/XQParser.y"
    {
    LOCATION((yylsp[(1) - (2)]), loc);
    CONTEXT->addSchemaLocation((yyvsp[(2) - (2)].str), 0, &loc);
  }
    break;

  case 101:
#line 1402 "../src/parser/XQParser.y"
    {
    VectorOfStrings schemaLoc(XQillaAllocator<const XMLCh*>(MEMMGR));
    schemaLoc.push_back((yyvsp[(2) - (2)].str));

    LOCATION((yylsp[(1) - (2)]), loc);
    CONTEXT->addSchemaLocation(XMLUni::fgZeroLenString, &schemaLoc, &loc);
  }
    break;

  case 102:
#line 1410 "../src/parser/XQParser.y"
    {
    VectorOfStrings schemaLoc(XQillaAllocator<const XMLCh*>(MEMMGR));
    schemaLoc.push_back((yyvsp[(3) - (3)].str));

    LOCATION((yylsp[(1) - (3)]), loc);
    CONTEXT->addSchemaLocation((yyvsp[(2) - (3)].str), &schemaLoc, &loc);
  }
    break;

  case 103:
#line 1418 "../src/parser/XQParser.y"
    {
    VectorOfStrings schemaLoc(XQillaAllocator<const XMLCh*>(MEMMGR));
    schemaLoc.push_back((yyvsp[(2) - (3)].str));

    LOCATION((yylsp[(1) - (3)]), loc);
    CONTEXT->addSchemaLocation((yyvsp[(3) - (3)].str), &schemaLoc, &loc);
  }
    break;

  case 104:
#line 1429 "../src/parser/XQParser.y"
    {
    VectorOfASTNodes *children = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    *children = (yyvsp[(3) - (4)].parenExpr)->getChildren();

    (yyval.astNode) = WRAP((yylsp[(1) - (4)]), new (MEMMGR) XQElementConstructor((yyvsp[(1) - (4)].astNode), (yyvsp[(2) - (4)].itemList), children, MEMMGR));
  }
    break;

  case 105:
#line 1439 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR)); 
  }
    break;

  case 106:
#line 1443 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = (yyvsp[(1) - (3)].itemList);

    ASTNode *attrItem = WRAP((yylsp[(2) - (3)]), new (MEMMGR) XQAttributeConstructor((yyvsp[(2) - (3)].astNode), (yyvsp[(3) - (3)].itemList), MEMMGR));
    (yyval.itemList)->push_back(attrItem);
  }
    break;

  case 107:
#line 1450 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = (yyvsp[(1) - (2)].itemList);
    (yyval.itemList)->insert((yyval.itemList)->begin(), (yyvsp[(2) - (2)].astNode));
  }
    break;

  case 108:
#line 1459 "../src/parser/XQParser.y"
    { 
    (yyval.itemList) = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
  }
    break;

  case 109:
#line 1463 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = (yyvsp[(1) - (2)].itemList);
    (yyval.itemList)->push_back(PRESERVE_NS((yylsp[(2) - (2)]), (yyvsp[(2) - (2)].astNode)));
  }
    break;

  case 110:
#line 1468 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = (yyvsp[(1) - (2)].itemList);
    (yyval.itemList)->push_back(WRAP((yylsp[(2) - (2)]), new (MEMMGR) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               SchemaSymbols::fgDT_STRING,
                               (yyvsp[(2) - (2)].str), AnyAtomicType::STRING, MEMMGR)));
  }
    break;

  case 111:
#line 1478 "../src/parser/XQParser.y"
    {
    (yyval.parenExpr) = WRAP((yyloc), new (MEMMGR) XQSequence(MEMMGR));
  }
    break;

  case 112:
#line 1482 "../src/parser/XQParser.y"
    {
    (yyval.parenExpr) = (yyvsp[(1) - (2)].parenExpr);
    (yyval.parenExpr)->addItem(WRAP((yylsp[(1) - (2)]), new (MEMMGR) XQTextConstructor((yyvsp[(2) - (2)].astNode), MEMMGR)));
  }
    break;

  case 113:
#line 1487 "../src/parser/XQParser.y"
    {
    (yyval.parenExpr) = (yyvsp[(1) - (2)].parenExpr);
    (yyval.parenExpr)->addItem((yyvsp[(2) - (2)].astNode));
  }
    break;

  case 114:
#line 1492 "../src/parser/XQParser.y"
    {
    (yyval.parenExpr) = (yyvsp[(1) - (2)].parenExpr);
    (yyval.parenExpr)->addItem((yyvsp[(2) - (2)].astNode));
  }
    break;

  case 133:
#line 1521 "../src/parser/XQParser.y"
    {
    XQTextConstructor *text = (XQTextConstructor*)(yyvsp[(1) - (3)].astNode);
    (yyval.astNode) = text;

    if(!(yyvsp[(2) - (3)].parenExpr)->getChildren().empty()) {
      if(text->getValue() != 0) {
        yyerror((yylsp[(1) - (3)]), "The xsl:value-of instruction has both a select attribute and a sequence constructor [err:XTSE0870]");
      }
      text->setValue((yyvsp[(2) - (3)].parenExpr));
    }
    else if(text->getValue() == 0) {
      yyerror((yylsp[(1) - (3)]), "The xsl:value-of instruction has neither a select attribute nor a sequence constructor [err:XTSE0870]");
    }
  }
    break;

  case 134:
#line 1539 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQTextConstructor(0, MEMMGR));
  }
    break;

  case 135:
#line 1543 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);
    ((XQTextConstructor*)(yyval.astNode))->setValue(PRESERVE_NS((yylsp[(2) - (3)]), (yyvsp[(3) - (3)].astNode)));
  }
    break;

  case 136:
#line 1548 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);
    // TBD separator - jpcs
/*     ((XQTextConstructor*)$$)->setValue(PRESERVE_NS(@2, $3)); */
  }
    break;

  case 137:
#line 1557 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQTextConstructor((yyvsp[(2) - (3)].astNode), MEMMGR));
  }
    break;

  case 140:
#line 1566 "../src/parser/XQParser.y"
    {
    // TBD xsl:sort - jpcs
    XQApplyTemplates *apply = (XQApplyTemplates*)(yyvsp[(1) - (3)].astNode);
    (yyval.astNode) = apply;

    apply->setArguments((yyvsp[(2) - (3)].templateArgs));

    if(apply->getExpression() == 0) {
      NodeTest *nt = new (MEMMGR) NodeTest();
      nt->setTypeWildcard();
      nt->setNameWildcard();
      nt->setNamespaceWildcard();

      apply->setExpression(WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQStep(XQStep::CHILD, nt, MEMMGR)));
    }
  }
    break;

  case 141:
#line 1586 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQApplyTemplates(0, 0, 0, MEMMGR));
  }
    break;

  case 142:
#line 1590 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);
    ((XQApplyTemplates*)(yyval.astNode))->setExpression(PRESERVE_NS((yylsp[(2) - (3)]), (yyvsp[(3) - (3)].astNode)));
  }
    break;

  case 143:
#line 1595 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);
    ((XQApplyTemplates*)(yyval.astNode))->setMode((yyvsp[(3) - (3)].mode));
  }
    break;

  case 144:
#line 1603 "../src/parser/XQParser.y"
    {
    RESOLVE_QNAME((yylsp[(1) - (1)]), (yyvsp[(1) - (1)].str));
    (yyval.mode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQUserFunction::Mode(uri, name));
  }
    break;

  case 145:
#line 1608 "../src/parser/XQParser.y"
    {
    (yyval.mode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::DEFAULT));
  }
    break;

  case 146:
#line 1612 "../src/parser/XQParser.y"
    {
    (yyval.mode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::CURRENT));
  }
    break;

  case 147:
#line 1619 "../src/parser/XQParser.y"
    {
    (yyval.templateArgs) = new (MEMMGR) TemplateArguments(XQillaAllocator<XQTemplateArgument*>(MEMMGR));
  }
    break;

  case 148:
#line 1623 "../src/parser/XQParser.y"
    {
    (yyval.templateArgs) = (yyvsp[(1) - (2)].templateArgs);
    (yyval.templateArgs)->push_back((yyvsp[(2) - (2)].templateArg));
  }
    break;

  case 149:
#line 1631 "../src/parser/XQParser.y"
    {
    XQCallTemplate *call = (XQCallTemplate*)(yyvsp[(1) - (3)].astNode);
    call->setArguments((yyvsp[(2) - (3)].templateArgs));
    (yyval.astNode) = call;

    if(call->getName() == 0) {
      yyerror((yylsp[(1) - (3)]), "The xsl:call-template instruction does not have a {}name attribute");
    }
  }
    break;

  case 150:
#line 1644 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQCallTemplate((const XMLCh*)0, 0, MEMMGR));
  }
    break;

  case 151:
#line 1648 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (2)].astNode);
    RESOLVE_QNAME((yylsp[(2) - (2)]), (yyvsp[(2) - (2)].str));
    ((XQCallTemplate*)(yyval.astNode))->setURI(uri);
    ((XQCallTemplate*)(yyval.astNode))->setName(name);
  }
    break;

  case 152:
#line 1658 "../src/parser/XQParser.y"
    {
    (yyval.templateArgs) = new (MEMMGR) TemplateArguments(XQillaAllocator<XQTemplateArgument*>(MEMMGR));
  }
    break;

  case 153:
#line 1662 "../src/parser/XQParser.y"
    {
    (yyval.templateArgs) = (yyvsp[(1) - (2)].templateArgs);
    (yyval.templateArgs)->push_back((yyvsp[(2) - (2)].templateArg));
  }
    break;

  case 154:
#line 1670 "../src/parser/XQParser.y"
    {
    (yyval.templateArg) = (yyvsp[(1) - (3)].templateArg);

    if((yyval.templateArg)->name == 0) {
      yyerror((yylsp[(1) - (3)]), "The xsl:with-param instruction does not have a {}name attribute");
    }

    (yyvsp[(1) - (3)].templateArg)->value = XSLT_VARIABLE_VALUE((yylsp[(1) - (3)]), (yyvsp[(1) - (3)].templateArg)->value, (yyvsp[(2) - (3)].parenExpr), (yyvsp[(1) - (3)].templateArg)->seqType);
    (yyvsp[(1) - (3)].templateArg)->seqType = 0;

    if((yyvsp[(1) - (3)].templateArg)->value == 0) {
      yyerror((yylsp[(1) - (3)]), "The xsl:with-param instruction has both a select attribute and a sequence constructor [err:XTSE0870]");
    }
  }
    break;

  case 155:
#line 1688 "../src/parser/XQParser.y"
    {
    (yyval.templateArg) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQTemplateArgument(0, 0, MEMMGR));
  }
    break;

  case 156:
#line 1692 "../src/parser/XQParser.y"
    {
    (yyval.templateArg) = (yyvsp[(1) - (2)].templateArg);
    RESOLVE_QNAME((yylsp[(2) - (2)]), (yyvsp[(2) - (2)].str));
    (yyval.templateArg)->uri = uri;
    (yyval.templateArg)->name = name;
  }
    break;

  case 157:
#line 1699 "../src/parser/XQParser.y"
    {
    (yyval.templateArg) = (yyvsp[(1) - (3)].templateArg);
    (yyval.templateArg)->value = PRESERVE_NS((yylsp[(2) - (3)]), (yyvsp[(3) - (3)].astNode));
  }
    break;

  case 158:
#line 1704 "../src/parser/XQParser.y"
    {
    (yyval.templateArg) = (yyvsp[(1) - (3)].templateArg);
    (yyval.templateArg)->seqType = (yyvsp[(3) - (3)].sequenceType);
  }
    break;

  case 159:
#line 1709 "../src/parser/XQParser.y"
    {
    (yyval.templateArg) = (yyvsp[(1) - (2)].templateArg);
    // TBD tunnel parameters - jpcs
  }
    break;

  case 160:
#line 1717 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (2)].astNode);
  }
    break;

  case 161:
#line 1723 "../src/parser/XQParser.y"
    {
    // TBD xsl:fallback - jpcs
    (yyval.astNode) = PRESERVE_NS((yylsp[(2) - (3)]), (yyvsp[(3) - (3)].astNode));
  }
    break;

  case 162:
#line 1731 "../src/parser/XQParser.y"
    {
    ASTNode *empty = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQSequence(MEMMGR));
    (yyval.astNode) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQIf((yyvsp[(1) - (3)].astNode), (yyvsp[(2) - (3)].parenExpr), empty, MEMMGR));
  }
    break;

  case 163:
#line 1739 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = PRESERVE_NS((yylsp[(2) - (3)]), (yyvsp[(3) - (3)].astNode));
  }
    break;

  case 164:
#line 1746 "../src/parser/XQParser.y"
    {
    XQIf *iff = (XQIf*)(yyvsp[(2) - (4)].astNode);
    while(iff->getWhenFalse() != 0) {
      iff = (XQIf*)iff->getWhenFalse();
    }

    iff->setWhenFalse((yyvsp[(3) - (4)].astNode));

    (yyval.astNode) = (yyvsp[(2) - (4)].astNode);
  }
    break;

  case 165:
#line 1760 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (1)].astNode);
  }
    break;

  case 166:
#line 1764 "../src/parser/XQParser.y"
    {
    XQIf *iff = (XQIf*)(yyvsp[(1) - (2)].astNode);
    while(iff->getWhenFalse() != 0) {
      iff = (XQIf*)iff->getWhenFalse();
    }

    iff->setWhenFalse((yyvsp[(2) - (2)].astNode));

    (yyval.astNode) = (yyvsp[(1) - (2)].astNode);
  }
    break;

  case 167:
#line 1778 "../src/parser/XQParser.y"
    {
    XQIf *iff = (XQIf*)(yyvsp[(1) - (3)].astNode);
    iff->setWhenTrue((yyvsp[(2) - (3)].parenExpr));
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);
  }
    break;

  case 168:
#line 1787 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQIf(PRESERVE_NS((yylsp[(2) - (3)]), (yyvsp[(3) - (3)].astNode)), 0, 0, MEMMGR));
  }
    break;

  case 169:
#line 1794 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yyloc), new (MEMMGR) XQSequence(MEMMGR));
  }
    break;

  case 170:
#line 1798 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(2) - (3)].parenExpr);
  }
    break;

  case 171:
#line 1805 "../src/parser/XQParser.y"
    {
    // TBD xsl:fallback - jpcs

    XQAnalyzeString *as = (XQAnalyzeString*)(yyval.astNode);
    (yyval.astNode) = (yyvsp[(1) - (4)].astNode);

    if(as->getExpression() == 0) {
      yyerror((yylsp[(1) - (4)]), "The xsl:analyze-string instruction does not have a {}select attribute");
    }

    if(as->getRegex() == 0) {
      yyerror((yylsp[(1) - (4)]), "The xsl:analyze-string instruction does not have a {}regex attribute");
    }

    if((yyvsp[(2) - (4)].astNode) == 0) {
      if((yyvsp[(3) - (4)].astNode) == 0) {
        yyerror((yylsp[(1) - (4)]), "The xsl:analyze-string instruction doesn't contain an xsl:matching-substring or xsl:non-matching-substring child [err:XTSE1130]");
      }

      as->setMatch(WRAP((yylsp[(1) - (4)]), new (MEMMGR) XQSequence(MEMMGR)));
    }
    else {
      as->setMatch((yyvsp[(2) - (4)].astNode));
    }
    if((yyvsp[(3) - (4)].astNode) == 0) {
      as->setNonMatch(WRAP((yylsp[(1) - (4)]), new (MEMMGR) XQSequence(MEMMGR)));
    }
    else {
      as->setNonMatch((yyvsp[(3) - (4)].astNode));
    }
  }
    break;

  case 172:
#line 1840 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yyloc), new (MEMMGR) XQAnalyzeString(MEMMGR));
  }
    break;

  case 173:
#line 1844 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);
    ((XQAnalyzeString*)(yyval.astNode))->setExpression(PRESERVE_NS((yylsp[(2) - (3)]), (yyvsp[(3) - (3)].astNode)));
  }
    break;

  case 174:
#line 1849 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);
    ASTNode *content = WRAP((yylsp[(3) - (3)]), new (MEMMGR) XQSimpleContent((yyvsp[(3) - (3)].itemList), MEMMGR));
    ((XQAnalyzeString*)(yyval.astNode))->setRegex(PRESERVE_NS((yylsp[(2) - (3)]), content));
  }
    break;

  case 175:
#line 1855 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);
    ASTNode *content = WRAP((yylsp[(3) - (3)]), new (MEMMGR) XQSimpleContent((yyvsp[(3) - (3)].itemList), MEMMGR));
    ((XQAnalyzeString*)(yyval.astNode))->setFlags(PRESERVE_NS((yylsp[(2) - (3)]), content));
  }
    break;

  case 176:
#line 1864 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = 0;
  }
    break;

  case 177:
#line 1868 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(2) - (3)].parenExpr);
  }
    break;

  case 178:
#line 1875 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = 0;
  }
    break;

  case 179:
#line 1879 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(2) - (3)].parenExpr);
  }
    break;

  case 180:
#line 1886 "../src/parser/XQParser.y"
    {
    (yyvsp[(1) - (4)].letTuple)->setExpression(XSLT_VARIABLE_VALUE((yylsp[(1) - (4)]), (yyvsp[(1) - (4)].letTuple)->getExpression(), (yyvsp[(2) - (4)].parenExpr), (yyvsp[(1) - (4)].letTuple)->seqType));
    (yyvsp[(1) - (4)].letTuple)->seqType = 0;

    if((yyvsp[(1) - (4)].letTuple)->getExpression() == 0) {
      yyerror((yylsp[(1) - (4)]), "The xsl:variable instruction has both a select attribute and a sequence constructor [err:XTSE0620]");
    }

    if((yyvsp[(1) - (4)].letTuple)->getVarName() == 0) {
      yyerror((yylsp[(1) - (4)]), "The xsl:variable instruction does not have a {}name attribute");
    }

    (yyval.astNode) = WRAP((yylsp[(1) - (4)]), new (MEMMGR) XQReturn((yyvsp[(1) - (4)].letTuple), (yyvsp[(4) - (4)].parenExpr), MEMMGR));
  }
    break;

  case 181:
#line 1904 "../src/parser/XQParser.y"
    {
    (yyval.letTuple) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) LetTuple(WRAP((yylsp[(1) - (1)]), new (MEMMGR) ContextTuple(MEMMGR)), 0, 0, MEMMGR));
  }
    break;

  case 182:
#line 1908 "../src/parser/XQParser.y"
    {
    RESOLVE_QNAME((yylsp[(2) - (2)]), (yyvsp[(2) - (2)].str));
    (yyvsp[(1) - (2)].letTuple)->setVarURI(uri);
    (yyvsp[(1) - (2)].letTuple)->setVarName(name);
    (yyval.letTuple) = (yyvsp[(1) - (2)].letTuple);
  }
    break;

  case 183:
#line 1915 "../src/parser/XQParser.y"
    {
    (yyvsp[(1) - (3)].letTuple)->setExpression((yyvsp[(3) - (3)].astNode));
    (yyval.letTuple) = (yyvsp[(1) - (3)].letTuple);
  }
    break;

  case 184:
#line 1920 "../src/parser/XQParser.y"
    {
    (yyvsp[(1) - (3)].letTuple)->seqType = (yyvsp[(3) - (3)].sequenceType);
    (yyval.letTuple) = (yyvsp[(1) - (3)].letTuple);
  }
    break;

  case 185:
#line 1928 "../src/parser/XQParser.y"
    {
    XQCommentConstructor *comment = (XQCommentConstructor*)(yyvsp[(1) - (3)].astNode);
    (yyval.astNode) = comment;

    if(!(yyvsp[(2) - (3)].parenExpr)->getChildren().empty()) {
      if(comment->getValue() != 0) {
        yyerror((yylsp[(1) - (3)]), "The xsl:comment instruction has both a select attribute and a sequence constructor [err:XTSE0940]");
      }
      comment->setValue((yyvsp[(2) - (3)].parenExpr));
    }
    else if(comment->getValue() == 0) {
      comment->setValue(WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQSequence(MEMMGR)));
    }
  }
    break;

  case 186:
#line 1946 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQCommentConstructor(0, MEMMGR, /*xslt*/true));
  }
    break;

  case 187:
#line 1950 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);
    ((XQCommentConstructor*)(yyval.astNode))->setValue(PRESERVE_NS((yylsp[(2) - (3)]), (yyvsp[(3) - (3)].astNode)));
  }
    break;

  case 188:
#line 1958 "../src/parser/XQParser.y"
    {
    XQPIConstructor *pi = (XQPIConstructor*)(yyvsp[(1) - (3)].astNode);
    (yyval.astNode) = pi;

    if(pi->getName() == 0) {
      yyerror((yylsp[(1) - (3)]), "The xsl:processing-instruction instruction does not have a {}name attribute");
    }

    if(!(yyvsp[(2) - (3)].parenExpr)->getChildren().empty()) {
      if(pi->getValue() != 0) {
        yyerror((yylsp[(1) - (3)]), "The xsl:processing-instruction instruction has both a select attribute and a sequence constructor [err:XTSE0880]");
      }
      pi->setValue((yyvsp[(2) - (3)].parenExpr));
    }
    else if(pi->getValue() == 0) {
      pi->setValue(WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQSequence(MEMMGR)));
    }
  }
    break;

  case 189:
#line 1980 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQPIConstructor(0, 0, MEMMGR, /*xslt*/true));
  }
    break;

  case 190:
#line 1984 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);

    ASTNode *content = WRAP((yylsp[(3) - (3)]), new (MEMMGR) XQSimpleContent((yyvsp[(3) - (3)].itemList), MEMMGR));
    ((XQPIConstructor*)(yyval.astNode))->setName(PRESERVE_NS((yylsp[(2) - (3)]), content));
  }
    break;

  case 191:
#line 1991 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);
    ((XQPIConstructor*)(yyval.astNode))->setValue(PRESERVE_NS((yylsp[(2) - (3)]), (yyvsp[(3) - (3)].astNode)));
  }
    break;

  case 192:
#line 1999 "../src/parser/XQParser.y"
    {
    XQDocumentConstructor *doc = (XQDocumentConstructor*)(yyvsp[(1) - (3)].astNode);
    doc->setValue((yyvsp[(2) - (3)].parenExpr));
    (yyval.astNode) = doc;
  }
    break;

  case 193:
#line 2008 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQDocumentConstructor(0, MEMMGR));
  }
    break;

  case 194:
#line 2016 "../src/parser/XQParser.y"
    {
    XQAttributeConstructor *attr = (XQAttributeConstructor*)(yyvsp[(1) - (3)].astNode);
    (yyval.astNode) = attr;

    if(attr->getName() == 0) {
      yyerror((yylsp[(1) - (3)]), "The xsl:attribute instruction does not have a {}name attribute");
    }

    if(attr->namespaceExpr != 0) {
      // Use fn:QName() to assign the correct URI
      VectorOfASTNodes args(XQillaAllocator<ASTNode*>(MEMMGR));
      args.push_back(attr->namespaceExpr);
      args.push_back(const_cast<ASTNode*>(attr->getName()));
      FunctionQName *name = WRAP((yylsp[(1) - (3)]), new (MEMMGR) FunctionQName(args, MEMMGR));
      attr->setName(name);
      attr->namespaceExpr = 0;
    }

    if(!(yyvsp[(2) - (3)].parenExpr)->getChildren().empty()) {
      if(attr->getChildren() != 0 && !attr->getChildren()->empty()) {
        yyerror((yylsp[(1) - (3)]), "The xsl:attribute instruction has both a select attribute and a sequence constructor [err:XTSE0840]");
      }

      VectorOfASTNodes *children = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
      *children = (yyvsp[(2) - (3)].parenExpr)->getChildren();
      attr->setChildren(children);
    }
    else if(attr->getChildren() == 0) {
      VectorOfASTNodes *children = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
      children->push_back(WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQSequence(MEMMGR)));
      attr->setChildren(children);
    }
  }
    break;

  case 195:
#line 2053 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQAttributeConstructor(0, 0, MEMMGR));
  }
    break;

  case 196:
#line 2057 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);

    ASTNode *content = WRAP((yylsp[(3) - (3)]), new (MEMMGR) XQSimpleContent((yyvsp[(3) - (3)].itemList), MEMMGR));
    ((XQAttributeConstructor*)(yyval.astNode))->setName(PRESERVE_NS((yylsp[(2) - (3)]), content));
  }
    break;

  case 197:
#line 2064 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);

    ASTNode *content = WRAP((yylsp[(3) - (3)]), new (MEMMGR) XQSimpleContent((yyvsp[(3) - (3)].itemList), MEMMGR));
    ((XQAttributeConstructor*)(yyval.astNode))->namespaceExpr = PRESERVE_NS((yylsp[(2) - (3)]), content);
  }
    break;

  case 198:
#line 2071 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);

    VectorOfASTNodes *children = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    children->push_back(PRESERVE_NS((yylsp[(2) - (3)]), (yyvsp[(3) - (3)].astNode)));

    ((XQAttributeConstructor*)(yyval.astNode))->setChildren(children);
  }
    break;

  case 199:
#line 2080 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);
    // TBD separator - jpcs
/*     ((XQAttributeConstructor*)$$)->setChildren(children); */
  }
    break;

  case 200:
#line 2089 "../src/parser/XQParser.y"
    {
    XQNamespaceConstructor *ns = (XQNamespaceConstructor*)(yyvsp[(1) - (3)].astNode);
    (yyval.astNode) = ns;

    if(ns->getName() == 0) {
      yyerror((yylsp[(1) - (3)]), "The xsl:namespace instruction does not have a {}name attribute");
    }

    if(!(yyvsp[(2) - (3)].parenExpr)->getChildren().empty()) {
      if(ns->getChildren() != 0 && !ns->getChildren()->empty()) {
        yyerror((yylsp[(1) - (3)]), "The xsl:namespace instruction has both a select attribute and a sequence constructor [err:XTSE0840]");
      }

      VectorOfASTNodes *children = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
      *children = (yyvsp[(2) - (3)].parenExpr)->getChildren();
      ns->setChildren(children);
    }
    else if(ns->getChildren() == 0) {
      VectorOfASTNodes *children = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
      children->push_back(WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQSequence(MEMMGR)));
      ns->setChildren(children);
    }
  }
    break;

  case 201:
#line 2116 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQNamespaceConstructor(0, 0, MEMMGR));
  }
    break;

  case 202:
#line 2120 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);

    ASTNode *content = WRAP((yylsp[(3) - (3)]), new (MEMMGR) XQSimpleContent((yyvsp[(3) - (3)].itemList), MEMMGR));
    ((XQNamespaceConstructor*)(yyval.astNode))->setName(PRESERVE_NS((yylsp[(2) - (3)]), content));
  }
    break;

  case 203:
#line 2127 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);

    VectorOfASTNodes *children = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    children->push_back(PRESERVE_NS((yylsp[(2) - (3)]), (yyvsp[(3) - (3)].astNode)));

    ((XQNamespaceConstructor*)(yyval.astNode))->setChildren(children);
  }
    break;

  case 204:
#line 2139 "../src/parser/XQParser.y"
    {
    XQElementConstructor *elem = (XQElementConstructor*)(yyvsp[(1) - (3)].astNode);
    (yyval.astNode) = elem;

    if(elem->getName() == 0) {
      yyerror((yylsp[(1) - (3)]), "The xsl:element instruction does not have a {}name attribute");
    }

    if(elem->namespaceExpr != 0) {
      // Use fn:QName() to assign the correct URI
      VectorOfASTNodes args(XQillaAllocator<ASTNode*>(MEMMGR));
      args.push_back(elem->namespaceExpr);
      args.push_back(const_cast<ASTNode*>(elem->getName()));
      FunctionQName *name = WRAP((yylsp[(1) - (3)]), new (MEMMGR) FunctionQName(args, MEMMGR));
      elem->setName(name);
      elem->namespaceExpr = 0;
    }

    VectorOfASTNodes *children = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    *children = (yyvsp[(2) - (3)].parenExpr)->getChildren();
    elem->setChildren(children);
  }
    break;

  case 205:
#line 2165 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQElementConstructor(0, 0, 0, MEMMGR));
  }
    break;

  case 206:
#line 2169 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);

    ASTNode *content = WRAP((yylsp[(3) - (3)]), new (MEMMGR) XQSimpleContent((yyvsp[(3) - (3)].itemList), MEMMGR));
    ((XQElementConstructor*)(yyval.astNode))->setName(PRESERVE_NS((yylsp[(2) - (3)]), content));
  }
    break;

  case 207:
#line 2176 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);

    ASTNode *content = WRAP((yylsp[(3) - (3)]), new (MEMMGR) XQSimpleContent((yyvsp[(3) - (3)].itemList), MEMMGR));
    ((XQElementConstructor*)(yyval.astNode))->namespaceExpr = PRESERVE_NS((yylsp[(2) - (3)]), content);
  }
    break;

  case 208:
#line 2186 "../src/parser/XQParser.y"
    {
    XQCopyOf *as = (XQCopyOf*)(yyval.astNode);
    (yyval.astNode) = (yyvsp[(1) - (2)].astNode);

    if(as->getExpression() == 0) {
      yyerror((yylsp[(1) - (2)]), "The xsl:copy-of instruction does not have a {}select attribute");
    }
  }
    break;

  case 209:
#line 2198 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yyloc), new (MEMMGR) XQCopyOf(MEMMGR));
  }
    break;

  case 210:
#line 2202 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);
    ((XQCopyOf*)(yyval.astNode))->setExpression(PRESERVE_NS((yylsp[(2) - (3)]), (yyvsp[(3) - (3)].astNode)));
  }
    break;

  case 211:
#line 2207 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (2)].astNode);
    ((XQCopyOf*)(yyval.astNode))->setCopyNamespaces((yyvsp[(2) - (2)].boolean));
  }
    break;

  case 212:
#line 2216 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);
    XQCopy *as = (XQCopy*)(yyval.astNode);

    XQContextItem *ci = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQContextItem(MEMMGR));
    as->setExpression(ci);

    as->setChildren((yyvsp[(2) - (3)].parenExpr)->getChildren());
  }
    break;

  case 213:
#line 2229 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yyloc), new (MEMMGR) XQCopy(MEMMGR));
  }
    break;

  case 214:
#line 2233 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (2)].astNode);
    ((XQCopy*)(yyval.astNode))->setCopyNamespaces((yyvsp[(2) - (2)].boolean));
  }
    break;

  case 215:
#line 2238 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (2)].astNode);
    ((XQCopy*)(yyval.astNode))->setInheritNamespaces((yyvsp[(2) - (2)].boolean));
  }
    break;

  case 216:
#line 2247 "../src/parser/XQParser.y"
    {
    // TBD xsl:sort - jpcs
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);
    XQMap *map = (XQMap*)(yyval.astNode);
    map->setArg2((yyvsp[(2) - (3)].parenExpr));

    if(map->getArg1() == 0) {
      yyerror((yylsp[(1) - (3)]), "The xsl:for-each instruction does not have a {}select attribute");
    }
  }
    break;

  case 217:
#line 2261 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQMap(0, 0, MEMMGR));
  }
    break;

  case 218:
#line 2265 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (3)].astNode);
    ((XQMap*)(yyval.astNode))->setArg1(PRESERVE_NS((yylsp[(2) - (3)]), (yyvsp[(3) - (3)].astNode)));
  }
    break;

  case 219:
#line 2278 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = (yyvsp[(1) - (3)].itemList);
    (yyval.itemList)->push_back(PRESERVE_NS((yylsp[(3) - (3)]), (yyvsp[(3) - (3)].astNode)));
  }
    break;

  case 220:
#line 2283 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    (yyval.itemList)->push_back(PRESERVE_NS((yylsp[(1) - (1)]), (yyvsp[(1) - (1)].astNode)));
  }
    break;

  case 221:
#line 2295 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (1)].astNode);
  }
    break;

  case 222:
#line 2299 "../src/parser/XQParser.y"
    {
    NodeTest *nt = new (MEMMGR) NodeTest();
    nt->setNodeType(Node::document_string);
    nt->setNameWildcard();
    nt->setNamespaceWildcard();
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQStep(XQStep::SELF, nt, MEMMGR));
  }
    break;

  case 223:
#line 2307 "../src/parser/XQParser.y"
    {
    // . intersect id("a")

    VectorOfASTNodes oargs(XQillaAllocator<ASTNode*>(MEMMGR));
    oargs.push_back(WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQContextItem(MEMMGR)));
    oargs.push_back((yyvsp[(1) - (1)].astNode));

    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) Intersect(oargs, MEMMGR));
  }
    break;

  case 224:
#line 2320 "../src/parser/XQParser.y"
    {
    NodeTest *nt = new (MEMMGR) NodeTest();
    nt->setNodeType(Node::document_string);
    nt->setNameWildcard();
    nt->setNamespaceWildcard();
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQStep(XQStep::PARENT, nt, MEMMGR));
  }
    break;

  case 225:
#line 2328 "../src/parser/XQParser.y"
    {
    NodeTest *nt = new (MEMMGR) NodeTest();
    nt->setNodeType(Node::document_string);
    nt->setNameWildcard();
    nt->setNamespaceWildcard();
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQStep(XQStep::ANCESTOR, nt, MEMMGR));
  }
    break;

  case 226:
#line 2341 "../src/parser/XQParser.y"
    {
    VectorOfASTNodes fargs(XQillaAllocator<ASTNode*>(MEMMGR));
    fargs.push_back((yyvsp[(3) - (4)].astNode));

    (yyval.astNode) = WRAP((yylsp[(1) - (4)]), new (MEMMGR) FunctionId(fargs, MEMMGR));
  }
    break;

  case 227:
#line 2348 "../src/parser/XQParser.y"
    {
//     VectorOfASTNodes fargs(XQillaAllocator<ASTNode*>(MEMMGR));
//     fargs.push_back($3);
//     fargs.push_back($5);

//     $$ = WRAP(@1, new (MEMMGR) FunctionKey(fargs, MEMMGR));

    // TBD key() - jpcs
    (yyval.astNode) = WRAP((yylsp[(1) - (6)]), new (MEMMGR) XQContextItem(MEMMGR));
  }
    break;

  case 232:
#line 2387 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(1) - (1)].astNode);
  }
    break;

  case 233:
#line 2391 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (2)]), new (MEMMGR) XQPredicate((yyvsp[(2) - (2)].astNode), (yyvsp[(1) - (2)].astNode), MEMMGR));
  }
    break;

  case 234:
#line 2395 "../src/parser/XQParser.y"
    {
    // id("a")/foo -> self::foo[parent::node() intersect id("a")]

    NodeTest *nt = new (MEMMGR) NodeTest();
    nt->setTypeWildcard();
    nt->setNameWildcard();
    nt->setNamespaceWildcard();
    XQStep *step = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQStep(XQStep::PARENT, nt, MEMMGR));

    VectorOfASTNodes oargs(XQillaAllocator<ASTNode*>(MEMMGR));
    oargs.push_back(step);
    oargs.push_back((yyvsp[(1) - (3)].astNode));

    Intersect *intersect = WRAP((yylsp[(1) - (3)]), new (MEMMGR) Intersect(oargs, MEMMGR));

    (yyval.astNode) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQPredicate((yyvsp[(3) - (3)].astNode), intersect, MEMMGR));
  }
    break;

  case 235:
#line 2413 "../src/parser/XQParser.y"
    {
    // id("a")/foo -> self::foo[ancestor::node() intersect id("a")]

    NodeTest *nt = new (MEMMGR) NodeTest();
    nt->setTypeWildcard();
    nt->setNameWildcard();
    nt->setNamespaceWildcard();
    XQStep *step = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQStep(XQStep::ANCESTOR, nt, MEMMGR));

    VectorOfASTNodes oargs(XQillaAllocator<ASTNode*>(MEMMGR));
    oargs.push_back(step);
    oargs.push_back((yyvsp[(1) - (3)].astNode));

    Intersect *intersect = WRAP((yylsp[(1) - (3)]), new (MEMMGR) Intersect(oargs, MEMMGR));

    (yyval.astNode) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQPredicate((yyvsp[(3) - (3)].astNode), intersect, MEMMGR));
  }
    break;

  case 236:
#line 2431 "../src/parser/XQParser.y"
    {
    ASTNode *step = (yyvsp[(1) - (3)].astNode);
    while(step->getType() == ASTNode::PREDICATE)
      step = (ASTNode*)((XQPredicate*)step)->getExpression();

    ((XQStep*)step)->setAxis(XQStep::PARENT);

    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) XQPredicate((yyvsp[(3) - (3)].astNode), (yyvsp[(1) - (3)].astNode), MEMMGR));
  }
    break;

  case 237:
#line 2441 "../src/parser/XQParser.y"
    {
    ASTNode *step = (yyvsp[(1) - (3)].astNode);
    while(step->getType() == ASTNode::PREDICATE)
      step = (ASTNode*)((XQPredicate*)step)->getExpression();

    ((XQStep*)step)->setAxis(XQStep::ANCESTOR);

    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) XQPredicate((yyvsp[(3) - (3)].astNode), (yyvsp[(1) - (3)].astNode), MEMMGR));
  }
    break;

  case 238:
#line 2455 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = XQPredicate::addPredicates(WRAP((yylsp[(1) - (2)]), new (MEMMGR) XQStep(XQStep::SELF, (yyvsp[(1) - (2)].nodeTest), MEMMGR)), (yyvsp[(2) - (2)].predicates));
  }
    break;

  case 239:
#line 2463 "../src/parser/XQParser.y"
    {
    if(!(yyvsp[(1) - (1)].nodeTest)->isNodeTypeSet())
      (yyvsp[(1) - (1)].nodeTest)->setNodeType(Node::element_string);
    (yyval.nodeTest) = (yyvsp[(1) - (1)].nodeTest);
  }
    break;

  case 240:
#line 2469 "../src/parser/XQParser.y"
    {
    if(!(yyvsp[(3) - (3)].nodeTest)->isNodeTypeSet())
      (yyvsp[(3) - (3)].nodeTest)->setNodeType(Node::element_string);
    (yyval.nodeTest) = (yyvsp[(3) - (3)].nodeTest);
  }
    break;

  case 241:
#line 2475 "../src/parser/XQParser.y"
    {
    if(!(yyvsp[(3) - (3)].nodeTest)->isNodeTypeSet())
      (yyvsp[(3) - (3)].nodeTest)->setNodeType(Node::attribute_string);
    (yyval.nodeTest) = (yyvsp[(3) - (3)].nodeTest);
  }
    break;

  case 242:
#line 2481 "../src/parser/XQParser.y"
    {
    if(!(yyvsp[(2) - (2)].nodeTest)->isNodeTypeSet())
      (yyvsp[(2) - (2)].nodeTest)->setNodeType(Node::attribute_string);
    (yyval.nodeTest) = (yyvsp[(2) - (2)].nodeTest);
  }
    break;

  case 243:
#line 2492 "../src/parser/XQParser.y"
    {
    (yyval.predicates) = new (MEMMGR) VectorOfPredicates(MEMMGR);
  }
    break;

  case 244:
#line 2496 "../src/parser/XQParser.y"
    {
    XQPredicate *pred = WRAP((yylsp[(2) - (4)]), new (MEMMGR) XQPredicate((yyvsp[(3) - (4)].astNode), MEMMGR));
    (yyvsp[(1) - (4)].predicates)->push_back(pred);
    (yyval.predicates) = (yyvsp[(1) - (4)].predicates); 
  }
    break;

  case 252:
#line 2525 "../src/parser/XQParser.y"
    {
    // TBD Set the language correctly on the context - jpcs
    if(XPath2Utils::equals((yyvsp[(2) - (2)].str),sz1_0)) {
      QP->_lexer->setVersion3(false);
      QP->_query->setVersion3(false);
    }
    else if(XPath2Utils::equals((yyvsp[(2) - (2)].str),sz3_0)) {
      if(!QP->_lexer->isVersion3()) {
        yyerror((yylsp[(1) - (2)]), "This XQuery processor is not configured to support XQuery 3.0 [err:XQST0031]");
      }
      QP->_lexer->setVersion3(true);
      QP->_query->setVersion3(true);
    }
    else
      yyerror((yylsp[(1) - (2)]), "This XQuery processor only supports version 1.0 and 3.0 [err:XQST0031]");
  }
    break;

  case 253:
#line 2545 "../src/parser/XQParser.y"
    {
    XMLCh *encName = (yyvsp[(2) - (2)].str);
    if((*encName < chLatin_A || *encName > chLatin_Z) && (*encName < chLatin_a || *encName > chLatin_z))
      yyerror((yylsp[(1) - (2)]), "The specified encoding does not conform to the definition of EncName [err:XQST0087]");

    for(++encName; *encName; ++encName) {
      if((*encName < chLatin_A || *encName > chLatin_Z) &&
         (*encName < chLatin_a || *encName > chLatin_z) &&
         (*encName < chDigit_0 || *encName > chDigit_9) &&
         *encName != chPeriod && *encName != chDash)
        yyerror((yylsp[(1) - (2)]), "The specified encoding does not conform to the definition of EncName [err:XQST0087]");
    }
  }
    break;

  case 256:
#line 2569 "../src/parser/XQParser.y"
    {
    if(*(yyvsp[(5) - (6)].str) == 0)
      yyerror((yylsp[(5) - (6)]), "The literal that specifies the namespace of a module must not be of zero length [err:XQST0088]");
    QP->_query->setIsLibraryModule();
    QP->_query->setModuleTargetNamespace((yyvsp[(5) - (6)].str));
    SET_NAMESPACE((yylsp[(3) - (6)]), (yyvsp[(3) - (6)].str), (yyvsp[(5) - (6)].str));
  }
    break;

  case 258:
#line 2583 "../src/parser/XQParser.y"
    {
    CHECK_SECOND_STEP((yylsp[(2) - (3)]), "a setter");
  }
    break;

  case 259:
#line 2587 "../src/parser/XQParser.y"
    {
    CHECK_SECOND_STEP((yylsp[(2) - (3)]), "an import");
  }
    break;

  case 260:
#line 2591 "../src/parser/XQParser.y"
    {
    CHECK_SECOND_STEP((yylsp[(2) - (3)]), "a namespace");
  }
    break;

  case 261:
#line 2595 "../src/parser/XQParser.y"
    {
    CHECK_SECOND_STEP((yylsp[(2) - (3)]), "a default namespace");
  }
    break;

  case 262:
#line 2599 "../src/parser/XQParser.y"
    {
    QP->_flags.set(BIT_DECLARE_SECOND_STEP);
  }
    break;

  case 263:
#line 2603 "../src/parser/XQParser.y"
    {
    QP->_flags.set(BIT_DECLARE_SECOND_STEP);
    XQUserFunction* decl=(yyvsp[(2) - (3)].functDecl);
    QP->_query->addFunction(decl);
  }
    break;

  case 264:
#line 2609 "../src/parser/XQParser.y"
    {
    QP->_flags.set(BIT_DECLARE_SECOND_STEP);
    XQUserFunction* decl=(yyvsp[(2) - (3)].functDecl);
    QP->_query->addFunction(decl);
  }
    break;

  case 265:
#line 2615 "../src/parser/XQParser.y"
    {
    QP->_flags.set(BIT_DECLARE_SECOND_STEP);
  }
    break;

  case 266:
#line 2619 "../src/parser/XQParser.y"
    {
    CHECK_SECOND_STEP((yylsp[(2) - (3)]), "an ftoption declaration");
  }
    break;

  case 278:
#line 2651 "../src/parser/XQParser.y"
    {
    SET_NAMESPACE((yylsp[(3) - (5)]), (yyvsp[(3) - (5)].str), (yyvsp[(5) - (5)].str));
  }
    break;

  case 279:
#line 2659 "../src/parser/XQParser.y"
    {
    CHECK_SPECIFIED((yylsp[(1) - (3)]), BIT_BOUNDARY_SPECIFIED, "boundary space", "XQST0068");
    CONTEXT->setPreserveBoundarySpace(true);
  }
    break;

  case 280:
#line 2664 "../src/parser/XQParser.y"
    {
    CHECK_SPECIFIED((yylsp[(1) - (3)]), BIT_BOUNDARY_SPECIFIED, "boundary space", "XQST0068");
    CONTEXT->setPreserveBoundarySpace(false);
  }
    break;

  case 281:
#line 2673 "../src/parser/XQParser.y"
    {
    CHECK_SPECIFIED((yylsp[(1) - (5)]), BIT_DEFAULTELEMENTNAMESPACE_SPECIFIED, "default element namespace", "XQST0066");
    CONTEXT->setDefaultElementAndTypeNS((yyvsp[(5) - (5)].str));
  }
    break;

  case 282:
#line 2678 "../src/parser/XQParser.y"
    { 
    CHECK_SPECIFIED((yylsp[(1) - (5)]), BIT_DEFAULTFUNCTIONNAMESPACE_SPECIFIED, "default function namespace", "XQST0066");
    CONTEXT->setDefaultFuncNS((yyvsp[(5) - (5)].str));
  }
    break;

  case 283:
#line 2687 "../src/parser/XQParser.y"
    {
    // validate the QName
    QualifiedName qName((yyvsp[(3) - (4)].str));
    const XMLCh* prefix = qName.getPrefix();
    if(prefix == 0 || *prefix == 0)
      yyerror((yylsp[(3) - (4)]), "The option name must have a prefix [err:XPST0081]");

    const XMLCh *uri = 0;
    try {
      LOCATION((yylsp[(3) - (4)]), loc);
      uri = CONTEXT->getUriBoundToPrefix(prefix, &loc);
    }
    catch(NamespaceLookupException&) {
      yyerror((yylsp[(3) - (4)]), "The option name is using an undefined namespace prefix [err:XPST0081]");
    }

    if(XPath2Utils::equals(uri, XQillaFunction::XMLChFunctionURI)) {
      if(XPath2Utils::equals(qName.getName(), option_projection)) {
        if(XPath2Utils::equals((yyvsp[(4) - (4)].str), SchemaSymbols::fgATTVAL_TRUE)) {
          CONTEXT->setProjection(true);
        }
        else if(XPath2Utils::equals((yyvsp[(4) - (4)].str), SchemaSymbols::fgATTVAL_FALSE)) {
          CONTEXT->setProjection(false);
        }
        else {
          yyerror((yylsp[(4) - (4)]), "Unknown value for option xqilla:projection. Should be 'true' or 'false' [err:XQILLA]");
        }
      }
      else if(XPath2Utils::equals(qName.getName(), option_psvi)) {
        if(XPath2Utils::equals((yyvsp[(4) - (4)].str), SchemaSymbols::fgATTVAL_TRUE)) {
          CONTEXT->getDocumentCache()->setDoPSVI(true);
        }
        else if(XPath2Utils::equals((yyvsp[(4) - (4)].str), SchemaSymbols::fgATTVAL_FALSE)) {
          CONTEXT->getDocumentCache()->setDoPSVI(false);
        }
        else {
          yyerror((yylsp[(4) - (4)]), "Unknown value for option xqilla:psvi. Should be 'true' or 'false' [err:XQILLA]");
        }
      }
      else if(XPath2Utils::equals(qName.getName(), option_lint)) {
        if(XPath2Utils::equals((yyvsp[(4) - (4)].str), SchemaSymbols::fgATTVAL_TRUE)) {
          CONTEXT->setDoLintWarnings(true);
        }
        else if(XPath2Utils::equals((yyvsp[(4) - (4)].str), SchemaSymbols::fgATTVAL_FALSE)) {
          CONTEXT->setDoLintWarnings(false);
        }
        else {
          yyerror((yylsp[(4) - (4)]), "Unknown value for option xqilla:lint. Should be 'true' or 'false' [err:XQILLA]");
        }
      }
      else {
        yyerror((yylsp[(3) - (4)]), "Unknown option name in the xqilla namespace [err:XQILLA]");
      }
    }
  }
    break;

  case 284:
#line 2747 "../src/parser/XQParser.y"
    {
    // TBD FTOptionDecl
  }
    break;

  case 285:
#line 2755 "../src/parser/XQParser.y"
    {
    CHECK_SPECIFIED((yylsp[(1) - (3)]), BIT_ORDERING_SPECIFIED, "ordering mode", "XQST0065");
    CONTEXT->setNodeSetOrdering(StaticContext::ORDERING_ORDERED);
  }
    break;

  case 286:
#line 2760 "../src/parser/XQParser.y"
    {
    CHECK_SPECIFIED((yylsp[(1) - (3)]), BIT_ORDERING_SPECIFIED, "ordering mode", "XQST0065");
    CONTEXT->setNodeSetOrdering(StaticContext::ORDERING_UNORDERED);
  }
    break;

  case 287:
#line 2769 "../src/parser/XQParser.y"
    { 
    CHECK_SPECIFIED((yylsp[(1) - (5)]), BIT_EMPTYORDERING_SPECIFIED, "empty ordering mode", "XQST0069");
    CONTEXT->setDefaultFLWOROrderingMode(StaticContext::FLWOR_ORDER_EMPTY_GREATEST);
  }
    break;

  case 288:
#line 2774 "../src/parser/XQParser.y"
    { 
    CHECK_SPECIFIED((yylsp[(1) - (5)]), BIT_EMPTYORDERING_SPECIFIED, "empty ordering mode", "XQST0069");
    CONTEXT->setDefaultFLWOROrderingMode(StaticContext::FLWOR_ORDER_EMPTY_LEAST);
  }
    break;

  case 289:
#line 2783 "../src/parser/XQParser.y"
    {
    CHECK_SPECIFIED((yylsp[(1) - (5)]), BIT_COPYNAMESPACE_SPECIFIED, "copy namespace", "XQST0055");
    CONTEXT->setPreserveNamespaces((yyvsp[(3) - (5)].boolean));
    CONTEXT->setInheritNamespaces((yyvsp[(5) - (5)].boolean));
  }
    break;

  case 290:
#line 2793 "../src/parser/XQParser.y"
    {
    (yyval.boolean) = true;
  }
    break;

  case 291:
#line 2797 "../src/parser/XQParser.y"
    {
    (yyval.boolean) = false;
  }
    break;

  case 292:
#line 2805 "../src/parser/XQParser.y"
    {
    (yyval.boolean) = true;
  }
    break;

  case 293:
#line 2809 "../src/parser/XQParser.y"
    {
    (yyval.boolean) = false;
  }
    break;

  case 294:
#line 2817 "../src/parser/XQParser.y"
    {
    CHECK_SPECIFIED((yylsp[(1) - (4)]), BIT_COLLATION_SPECIFIED, "default collation", "XQST0038");
    try {
      LOCATION((yylsp[(4) - (4)]), loc);
      CONTEXT->getCollation((yyvsp[(4) - (4)].str), &loc);
    }
    catch(ContextException&) {
      yyerror((yylsp[(4) - (4)]), "The specified collation does not exist [err:XQST0038]");
    }
    CONTEXT->setDefaultCollation((yyvsp[(4) - (4)].str));
  }
    break;

  case 295:
#line 2833 "../src/parser/XQParser.y"
    {
    CHECK_SPECIFIED((yylsp[(1) - (3)]), BIT_BASEURI_SPECIFIED, "base URI", "XQST0032");
    CONTEXT->setBaseURI((yyvsp[(3) - (3)].str));
  }
    break;

  case 296:
#line 2842 "../src/parser/XQParser.y"
    {
    if(XPath2Utils::equals((yyvsp[(3) - (5)].str), XMLUni::fgZeroLenString))
      CONTEXT->setDefaultElementAndTypeNS((yyvsp[(4) - (5)].str));
    else if(XPath2Utils::equals((yyvsp[(4) - (5)].str), XMLUni::fgZeroLenString))
      yyerror((yylsp[(1) - (5)]), "A schema that has no target namespace cannot be bound to a non-empty prefix [err:XQST0057]");
    else {
      SET_NAMESPACE((yylsp[(3) - (5)]), (yyvsp[(3) - (5)].str), (yyvsp[(4) - (5)].str));
    }
    LOCATION((yylsp[(1) - (5)]), loc);
    CONTEXT->addSchemaLocation((yyvsp[(4) - (5)].str), (yyvsp[(5) - (5)].stringList), &loc);
  }
    break;

  case 297:
#line 2854 "../src/parser/XQParser.y"
    {
    LOCATION((yylsp[(1) - (4)]), loc);
    CONTEXT->addSchemaLocation((yyvsp[(3) - (4)].str), (yyvsp[(4) - (4)].stringList), &loc);
  }
    break;

  case 298:
#line 2862 "../src/parser/XQParser.y"
    {
    (yyval.stringList) = NULL;
  }
    break;

  case 299:
#line 2866 "../src/parser/XQParser.y"
    {
    (yyval.stringList) = new (MEMMGR) VectorOfStrings(XQillaAllocator<const XMLCh*>(MEMMGR));
    (yyval.stringList)->push_back((yyvsp[(2) - (2)].str));
  }
    break;

  case 300:
#line 2871 "../src/parser/XQParser.y"
    {
    (yyvsp[(1) - (3)].stringList)->push_back((yyvsp[(3) - (3)].str));
    (yyval.stringList) = (yyvsp[(1) - (3)].stringList);
  }
    break;

  case 301:
#line 2880 "../src/parser/XQParser.y"
    {
    (yyval.str) = (yyvsp[(2) - (3)].str);
  }
    break;

  case 302:
#line 2884 "../src/parser/XQParser.y"
    {
    (yyval.str) = (XMLCh*)XMLUni::fgZeroLenString;
  }
    break;

  case 303:
#line 2892 "../src/parser/XQParser.y"
    {
    if(XMLString::stringLen((yyvsp[(6) - (7)].str))==0)
      yyerror((yylsp[(6) - (7)]), "The literal that specifies the target namespace in a module import must not be of zero length [err:XQST0088]");

    SET_NAMESPACE((yylsp[(4) - (7)]), (yyvsp[(4) - (7)].str), (yyvsp[(6) - (7)].str));

    LOCATION((yylsp[(1) - (7)]), loc);
    QP->_query->importModule((yyvsp[(6) - (7)].str), (yyvsp[(7) - (7)].stringList), &loc);
  }
    break;

  case 304:
#line 2902 "../src/parser/XQParser.y"
    {
    if(XMLString::stringLen((yyvsp[(3) - (4)].str))==0)
      yyerror((yylsp[(3) - (4)]), "The literal that specifies the target namespace in a module import must not be of zero length [err:XQST0088]");

    LOCATION((yylsp[(1) - (4)]), loc);
    QP->_query->importModule((yyvsp[(3) - (4)].str), (yyvsp[(4) - (4)].stringList), &loc);
  }
    break;

  case 305:
#line 2914 "../src/parser/XQParser.y"
    {
    QP->_query->addVariable(WRAP((yylsp[(1) - (6)]), new (MEMMGR) XQGlobalVariable((yyvsp[(4) - (6)].str), (yyvsp[(5) - (6)].sequenceType), (yyvsp[(6) - (6)].astNode), MEMMGR)));
  }
    break;

  case 306:
#line 2921 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(2) - (2)].astNode);
  }
    break;

  case 307:
#line 2925 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = NULL;
  }
    break;

  case 308:
#line 2933 "../src/parser/XQParser.y"
    {
    CHECK_SPECIFIED((yylsp[(1) - (3)]), BIT_CONSTRUCTION_SPECIFIED, "construction mode", "XQST0067");
    CONTEXT->setConstructionMode(StaticContext::CONSTRUCTION_MODE_PRESERVE);
  }
    break;

  case 309:
#line 2938 "../src/parser/XQParser.y"
    {
    CHECK_SPECIFIED((yylsp[(1) - (3)]), BIT_CONSTRUCTION_SPECIFIED, "construction mode", "XQST0067");
    CONTEXT->setConstructionMode(StaticContext::CONSTRUCTION_MODE_STRIP);
  }
    break;

  case 310:
#line 2947 "../src/parser/XQParser.y"
    {
    (yyvsp[(2) - (7)].signature)->argSpecs = (yyvsp[(5) - (7)].argSpecs);
    (yyvsp[(2) - (7)].signature)->returnType = (yyvsp[(6) - (7)].sequenceType);
    (yyval.functDecl) = WRAP((yylsp[(1) - (7)]), new (MEMMGR) XQUserFunction((yyvsp[(4) - (7)].str), (yyvsp[(2) - (7)].signature), (yyvsp[(7) - (7)].astNode), true, MEMMGR));
  }
    break;

  case 313:
#line 2958 "../src/parser/XQParser.y"
    {
    (yyval.argSpecs) = NULL;
  }
    break;

  case 314:
#line 2962 "../src/parser/XQParser.y"
    {
    (yyval.argSpecs) = (yyvsp[(2) - (3)].argSpecs);
  }
    break;

  case 316:
#line 2971 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = 0;
  }
    break;

  case 317:
#line 2979 "../src/parser/XQParser.y"
    {
    (yyval.signature) = new (MEMMGR) FunctionSignature(MEMMGR);
  }
    break;

  case 318:
#line 2983 "../src/parser/XQParser.y"
    {
    switch((yyvsp[(1) - (2)].signature)->privateOption) {
    case FunctionSignature::OP_TRUE:
      yyerror((yylsp[(2) - (2)]), "Function option 'private' already specified [err:XQST0106]");
    case FunctionSignature::OP_FALSE:
      yyerror((yylsp[(2) - (2)]), "Function option 'public' already specified [err:XQST0106]");
    case FunctionSignature::OP_DEFAULT:
      (yyvsp[(1) - (2)].signature)->privateOption = (yyvsp[(2) - (2)].boolean) ? FunctionSignature::OP_TRUE : FunctionSignature::OP_FALSE;
      break;
    }
    (yyval.signature) = (yyvsp[(1) - (2)].signature);
  }
    break;

  case 319:
#line 2996 "../src/parser/XQParser.y"
    {
    switch((yyvsp[(1) - (2)].signature)->nondeterministic) {
    case FunctionSignature::OP_TRUE:
      yyerror((yylsp[(2) - (2)]), "Function option 'nondeterministic' already specified [err:XQST0106]");
    case FunctionSignature::OP_FALSE:
      yyerror((yylsp[(2) - (2)]), "Function option 'deterministic' already specified [err:XQST0106]");
    case FunctionSignature::OP_DEFAULT:
      (yyvsp[(1) - (2)].signature)->nondeterministic = (yyvsp[(2) - (2)].boolean) ? FunctionSignature::OP_TRUE : FunctionSignature::OP_FALSE;
      break;
    }
    (yyval.signature) = (yyvsp[(1) - (2)].signature);
  }
    break;

  case 320:
#line 3009 "../src/parser/XQParser.y"
    {
    if((yyvsp[(1) - (2)].signature)->updating != FunctionSignature::OP_DEFAULT)
      yyerror((yylsp[(2) - (2)]), "Function option 'updating' already specified [err:XPST0003]");
    (yyvsp[(1) - (2)].signature)->updating = FunctionSignature::OP_TRUE;
    (yyval.signature) = (yyvsp[(1) - (2)].signature);
  }
    break;

  case 321:
#line 3018 "../src/parser/XQParser.y"
    { (yyval.boolean) = true; }
    break;

  case 322:
#line 3018 "../src/parser/XQParser.y"
    { (yyval.boolean) = false; }
    break;

  case 323:
#line 3021 "../src/parser/XQParser.y"
    { (yyval.boolean) = false; }
    break;

  case 324:
#line 3021 "../src/parser/XQParser.y"
    { (yyval.boolean) = true; }
    break;

  case 325:
#line 3025 "../src/parser/XQParser.y"
    {
    (yyval.sequenceType) = 0;
  }
    break;

  case 326:
#line 3029 "../src/parser/XQParser.y"
    {
    (yyval.sequenceType) = (yyvsp[(2) - (2)].sequenceType);
  }
    break;

  case 327:
#line 3037 "../src/parser/XQParser.y"
    {
    (yyvsp[(1) - (3)].argSpecs)->push_back((yyvsp[(3) - (3)].argSpec));
    (yyval.argSpecs) = (yyvsp[(1) - (3)].argSpecs);
  }
    break;

  case 328:
#line 3042 "../src/parser/XQParser.y"
    {
    ArgumentSpecs* paramList = new (MEMMGR) ArgumentSpecs(XQillaAllocator<ArgumentSpec*>(MEMMGR));
    paramList->push_back((yyvsp[(1) - (1)].argSpec));
    (yyval.argSpecs) = paramList;
  }
    break;

  case 329:
#line 3052 "../src/parser/XQParser.y"
    {
    (yyval.argSpec) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) ArgumentSpec((yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].sequenceType), MEMMGR));
  }
    break;

  case 330:
#line 3060 "../src/parser/XQParser.y"
    {
      (yyval.astNode) = (yyvsp[(2) - (3)].astNode);
    }
    break;

  case 331:
#line 3068 "../src/parser/XQParser.y"
    {
    if(QP->_lexer->isUpdate()) {
      QP->_query->setQueryBody(WRAP((yylsp[(1) - (1)]), new (MEMMGR) UApplyUpdates((yyvsp[(1) - (1)].astNode), MEMMGR)));
    }
    else {
      QP->_query->setQueryBody((yyvsp[(1) - (1)].astNode));
    }
  }
    break;

  case 332:
#line 3081 "../src/parser/XQParser.y"
    {
      ASTNode* prevExpr=(yyvsp[(1) - (3)].astNode);
      if(prevExpr->getType()==ASTNode::SEQUENCE)
      {
        ((XQSequence *)prevExpr)->addItem((yyvsp[(3) - (3)].astNode));
        (yyval.astNode) = (yyvsp[(1) - (3)].astNode);
      }
      else {
        XQSequence *dis = WRAP((yylsp[(2) - (3)]), new (MEMMGR) XQSequence(MEMMGR));
        dis->addItem((yyvsp[(1) - (3)].astNode));
        dis->addItem((yyvsp[(3) - (3)].astNode));
        (yyval.astNode) = dis;
      }
    }
    break;

  case 346:
#line 3119 "../src/parser/XQParser.y"
    {
      // Add a ContextTuple at the start
      TupleNode *tuples = setLastAncestor((yyvsp[(1) - (3)].tupleNode), WRAP((yylsp[(1) - (3)]), new (MEMMGR) ContextTuple(MEMMGR)));

      // Check the correct clause order for XQuery 1.0
      // FLWORExpr ::= (ForClause |  LetClause)+ WhereClause? OrderByClause? "return" ExprSingle
      if(!QP->_lexer->isVersion3()) {
        const TupleNode *where = 0;
        const TupleNode *forlet = 0;
        const TupleNode *node = tuples;
        while(node) {
          switch(node->getType()) {
          case TupleNode::ORDER_BY:
            if(where)
              XQThrow3(StaticErrorException, X("XQParser"), X("Where clause after order by clause [err:XPST0003]"), where);
            else if(forlet)
              XQThrow3(StaticErrorException, X("XQParser"), X("For or let clause after order by clause [err:XPST0003]"), forlet);
            break;
          case TupleNode::WHERE:
            if(where)
              XQThrow3(StaticErrorException, X("XQParser"), X("Duplicate where clause [err:XPST0003]"), where);
            else if(forlet)
              XQThrow3(StaticErrorException, X("XQParser"), X("For or let clause after where clause [err:XPST0003]"), forlet);
            where = node;
            break;
          case TupleNode::FOR:
          case TupleNode::LET:
            forlet = node;
            break;
          default:
            break;
          }

          node = node->getParent();
        }
      }

      // Add the return expression
      (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) XQReturn(tuples, (yyvsp[(3) - (3)].astNode), MEMMGR));
    }
    break;

  case 348:
#line 3164 "../src/parser/XQParser.y"
    {
    // Order by has a special check here, because a single OrderByClause can result
    // in multiple OrderByTuple objects.
    const TupleNode *node = (yyvsp[(1) - (2)].tupleNode);
    while(node) {
      if(node->getType() == TupleNode::ORDER_BY)
        XQThrow3(StaticErrorException, X("XQParser"), X("Duplicate order by clause [err:XPST0003]"), (yyvsp[(2) - (2)].tupleNode));
      node = node->getParent();
    }

    (yyval.tupleNode) = setLastAncestor((yyvsp[(2) - (2)].tupleNode), (yyvsp[(1) - (2)].tupleNode));
  }
    break;

  case 349:
#line 3177 "../src/parser/XQParser.y"
    {
    (yyval.tupleNode) = setLastAncestor((yyvsp[(2) - (2)].tupleNode), (yyvsp[(1) - (2)].tupleNode));
  }
    break;

  case 355:
#line 3192 "../src/parser/XQParser.y"
    {
      (yyval.tupleNode) = (yyvsp[(2) - (2)].tupleNode);
    }
    break;

  case 356:
#line 3199 "../src/parser/XQParser.y"
    {
      (yyval.tupleNode) = setLastAncestor((yyvsp[(3) - (3)].tupleNode), (yyvsp[(1) - (3)].tupleNode));
    }
    break;

  case 358:
#line 3207 "../src/parser/XQParser.y"
    {
    // the SequenceType has been specified for each item of the sequence, but we can only apply to the
    // sequence itself, so allow it to match multiple matches
    (yyvsp[(3) - (7)].sequenceType)->setOccurrence(SequenceType::STAR);
    (yyval.tupleNode) = WRAP((yylsp[(1) - (7)]), new (MEMMGR) ForTuple(0, (yyvsp[(2) - (7)].str), (yyvsp[(4) - (7)].str), WRAP((yylsp[(3) - (7)]), new (MEMMGR) XQTreatAs((yyvsp[(7) - (7)].astNode), (yyvsp[(3) - (7)].sequenceType), MEMMGR)), MEMMGR));
  }
    break;

  case 359:
#line 3218 "../src/parser/XQParser.y"
    {
    (yyval.str) = NULL;
  }
    break;

  case 360:
#line 3222 "../src/parser/XQParser.y"
    { 
    REJECT_NOT_XQUERY(PositionalVar, (yylsp[(1) - (3)]));

    (yyval.str) = (yyvsp[(3) - (3)].str); 
  }
    break;

  case 361:
#line 3232 "../src/parser/XQParser.y"
    {
    (yyval.str) = NULL;
  }
    break;

  case 362:
#line 3236 "../src/parser/XQParser.y"
    {
    (yyval.str) = (yyvsp[(3) - (3)].str);
  }
    break;

  case 363:
#line 3246 "../src/parser/XQParser.y"
    {
    (yyval.tupleNode) = (yyvsp[(2) - (2)].tupleNode);
  }
    break;

  case 364:
#line 3253 "../src/parser/XQParser.y"
    {
    (yyval.tupleNode) = setLastAncestor((yyvsp[(3) - (3)].tupleNode), (yyvsp[(1) - (3)].tupleNode));
  }
    break;

  case 366:
#line 3261 "../src/parser/XQParser.y"
    {
    (yyval.tupleNode) = WRAP((yylsp[(1) - (5)]), new (MEMMGR) LetTuple(0, (yyvsp[(2) - (5)].str), WRAP((yylsp[(3) - (5)]), new (MEMMGR) XQTreatAs((yyvsp[(5) - (5)].astNode), (yyvsp[(3) - (5)].sequenceType), MEMMGR)), MEMMGR));
  }
    break;

  case 367:
#line 3265 "../src/parser/XQParser.y"
    {
    ASTNode *literal = WRAP((yylsp[(1) - (5)]), new (MEMMGR) XQLiteral(
                  SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                  SchemaSymbols::fgDT_INTEGER,
                  X("0"), AnyAtomicType::DECIMAL,
                  MEMMGR));
    (yyval.tupleNode) = WRAP((yylsp[(1) - (5)]), new (MEMMGR) LetTuple(0, (yyvsp[(3) - (5)].str), literal, MEMMGR));
  }
    break;

  case 368:
#line 3278 "../src/parser/XQParser.y"
    { 
    (yyval.tupleNode) = WRAP((yylsp[(1) - (2)]), new (MEMMGR) WhereTuple(0, (yyvsp[(2) - (2)].astNode), MEMMGR));
  }
    break;

  case 369:
#line 3286 "../src/parser/XQParser.y"
    {
    (yyval.tupleNode) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) CountTuple(0, (yyvsp[(3) - (3)].str), MEMMGR));
  }
    break;

  case 370:
#line 3294 "../src/parser/XQParser.y"
    {
    ((OrderByTuple*)getLastAncestor((yyvsp[(3) - (3)].tupleNode)))->setUnstable();
    (yyval.tupleNode) = (yyvsp[(3) - (3)].tupleNode);
  }
    break;

  case 371:
#line 3299 "../src/parser/XQParser.y"
    {
    (yyval.tupleNode) = (yyvsp[(4) - (4)].tupleNode);
  }
    break;

  case 372:
#line 3307 "../src/parser/XQParser.y"
    {
    (yyval.tupleNode) = setLastAncestor((yyvsp[(1) - (3)].tupleNode), (yyvsp[(3) - (3)].tupleNode));
  }
    break;

  case 374:
#line 3317 "../src/parser/XQParser.y"
    {
    LOCATION((yylsp[(1) - (3)]), loc);
    Collation *collation = CONTEXT->getDefaultCollation(&loc);
    
    (yyval.tupleNode) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) OrderByTuple(0, (yyvsp[(1) - (3)].astNode), OrderByTuple::STABLE | (yyvsp[(2) - (3)].orderByModifier) | (yyvsp[(3) - (3)].orderByModifier), collation, MEMMGR));
  }
    break;

  case 375:
#line 3324 "../src/parser/XQParser.y"
    {
    try {
      LOCATION((yylsp[(4) - (5)]), loc);
      Collation *collation = CONTEXT->getCollation((yyvsp[(5) - (5)].str), &loc);

      (yyval.tupleNode) = WRAP((yylsp[(1) - (5)]), new (MEMMGR) OrderByTuple(0, (yyvsp[(1) - (5)].astNode), OrderByTuple::STABLE | (yyvsp[(2) - (5)].orderByModifier) | (yyvsp[(3) - (5)].orderByModifier), collation, MEMMGR));
    }
    catch(ContextException&) {
      yyerror((yylsp[(4) - (5)]), "The specified collation does not exist [err:XQST0076]");
    }
  }
    break;

  case 376:
#line 3339 "../src/parser/XQParser.y"
    {
    SequenceType *zero_or_one = WRAP((yylsp[(1) - (1)]), new (MEMMGR)
      SequenceType(new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ANYTHING),
                   SequenceType::QUESTION_MARK));

    (yyval.astNode) = (yyvsp[(1) - (1)].astNode);
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQAtomize((yyval.astNode), MEMMGR));
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQPromoteUntyped((yyval.astNode), SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                                SchemaSymbols::fgDT_STRING, MEMMGR));
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQTreatAs((yyval.astNode), zero_or_one, MEMMGR));
  }
    break;

  case 377:
#line 3355 "../src/parser/XQParser.y"
    {
    (yyval.orderByModifier) = OrderByTuple::ASCENDING;
  }
    break;

  case 378:
#line 3359 "../src/parser/XQParser.y"
    {
    (yyval.orderByModifier) = OrderByTuple::ASCENDING;
  }
    break;

  case 379:
#line 3363 "../src/parser/XQParser.y"
    {
    (yyval.orderByModifier) = OrderByTuple::DESCENDING;
  }
    break;

  case 380:
#line 3370 "../src/parser/XQParser.y"
    {
    switch(CONTEXT->getDefaultFLWOROrderingMode()) {
    case StaticContext::FLWOR_ORDER_EMPTY_LEAST:
      (yyval.orderByModifier) = OrderByTuple::EMPTY_LEAST; break;
    case StaticContext::FLWOR_ORDER_EMPTY_GREATEST:
      (yyval.orderByModifier) = OrderByTuple::EMPTY_GREATEST; break;
    }
  }
    break;

  case 381:
#line 3379 "../src/parser/XQParser.y"
    {
    (yyval.orderByModifier) = OrderByTuple::EMPTY_GREATEST;
  }
    break;

  case 382:
#line 3383 "../src/parser/XQParser.y"
    {
    (yyval.orderByModifier) = OrderByTuple::EMPTY_LEAST;
  }
    break;

  case 383:
#line 3392 "../src/parser/XQParser.y"
    {
    // Add a ContextTuple at the start
    TupleNode *tmp = setLastAncestor((yyvsp[(2) - (4)].tupleNode), WRAP((yylsp[(1) - (4)]), new (MEMMGR) ContextTuple(MEMMGR)));

    // Add the return expression
    (yyval.astNode) = WRAP((yylsp[(3) - (4)]), new (MEMMGR) XQQuantified(XQQuantified::SOME, tmp, (yyvsp[(4) - (4)].astNode), MEMMGR));
  }
    break;

  case 384:
#line 3400 "../src/parser/XQParser.y"
    {
    // Add a ContextTuple at the start
    TupleNode *tmp = setLastAncestor((yyvsp[(2) - (4)].tupleNode), WRAP((yylsp[(1) - (4)]), new (MEMMGR) ContextTuple(MEMMGR)));

    // Add the return expression
    (yyval.astNode) = WRAP((yylsp[(3) - (4)]), new (MEMMGR) XQQuantified(XQQuantified::EVERY, tmp, (yyvsp[(4) - (4)].astNode), MEMMGR));
  }
    break;

  case 385:
#line 3411 "../src/parser/XQParser.y"
    {
    (yyval.tupleNode) = setLastAncestor((yyvsp[(3) - (3)].tupleNode), (yyvsp[(1) - (3)].tupleNode));
  }
    break;

  case 387:
#line 3419 "../src/parser/XQParser.y"
    {
    // the SequenceType has been specified for each item of the sequence, but we can only apply to the
    // sequence itself, so allow it to match multiple matches
    (yyvsp[(3) - (5)].sequenceType)->setOccurrence(SequenceType::STAR);
    (yyval.tupleNode) = WRAP((yylsp[(1) - (5)]), new (MEMMGR) ForTuple(0, (yyvsp[(2) - (5)].str), 0, WRAP((yylsp[(3) - (5)]), new (MEMMGR) XQTreatAs((yyvsp[(5) - (5)].astNode), (yyvsp[(3) - (5)].sequenceType), MEMMGR)), MEMMGR));
  }
    break;

  case 388:
#line 3430 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (6)]), new (MEMMGR) XQTypeswitch( WRAP((yylsp[(1) - (6)]), (yyvsp[(3) - (6)].astNode)), (yyvsp[(5) - (6)].caseClauses), (yyvsp[(6) - (6)].caseClause), MEMMGR));
  }
    break;

  case 389:
#line 3437 "../src/parser/XQParser.y"
    {
    XQTypeswitch::Cases::iterator it = (yyvsp[(2) - (2)].caseClauses)->begin();
    for(; it != (yyvsp[(2) - (2)].caseClauses)->end(); ++it) {
      (yyvsp[(1) - (2)].caseClauses)->push_back(*it);
    }

    (yyval.caseClauses) = (yyvsp[(1) - (2)].caseClauses);
  }
    break;

  case 391:
#line 3450 "../src/parser/XQParser.y"
    {
    (yyval.caseClause) = WRAP((yylsp[(1) - (5)]), new (MEMMGR) XQTypeswitch::Case((yyvsp[(3) - (5)].str), NULL, (yyvsp[(5) - (5)].astNode)));
  }
    break;

  case 392:
#line 3454 "../src/parser/XQParser.y"
    {
    (yyval.caseClause) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQTypeswitch::Case(NULL, NULL, (yyvsp[(3) - (3)].astNode)));
  }
    break;

  case 393:
#line 3462 "../src/parser/XQParser.y"
    {
    // TBD Don't copy the expression when unions of SequenceTypes are implemented - jpcs
    XQTypeswitch::Cases::iterator it = (yyvsp[(1) - (3)].caseClauses)->begin();
    if(it != (yyvsp[(1) - (3)].caseClauses)->end()) {
      (*it)->setExpression((yyvsp[(3) - (3)].astNode));

      for(++it; it != (yyvsp[(1) - (3)].caseClauses)->end(); ++it) {
        (*it)->setExpression((yyvsp[(3) - (3)].astNode)->copy(CONTEXT));
      }
    }

    (yyval.caseClauses) = (yyvsp[(1) - (3)].caseClauses);
  }
    break;

  case 394:
#line 3480 "../src/parser/XQParser.y"
    {
    (yyval.caseClauses) = new (MEMMGR) XQTypeswitch::Cases(XQillaAllocator<XQTypeswitch::Case*>(MEMMGR));
    (yyval.caseClauses)->push_back(WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQTypeswitch::Case((yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].sequenceType), 0)));
  }
    break;

  case 395:
#line 3485 "../src/parser/XQParser.y"
    {
    REJECT_NOT_VERSION3(SequenceTypeUnion, (yylsp[(2) - (3)]));
    (yyvsp[(1) - (3)].caseClauses)->push_back(WRAP((yylsp[(3) - (3)]), new (MEMMGR) XQTypeswitch::Case((yyvsp[(1) - (3)].caseClauses)->back()->getQName(), (yyvsp[(3) - (3)].sequenceType), 0)));
    (yyval.caseClauses) = (yyvsp[(1) - (3)].caseClauses);
  }
    break;

  case 396:
#line 3494 "../src/parser/XQParser.y"
    {
    (yyval.str) = 0;
  }
    break;

  case 397:
#line 3498 "../src/parser/XQParser.y"
    {
    (yyval.str) = (yyvsp[(2) - (3)].str);
  }
    break;

  case 398:
#line 3506 "../src/parser/XQParser.y"
    { 
    (yyval.astNode) = WRAP((yylsp[(1) - (8)]), new (MEMMGR) XQIf((yyvsp[(3) - (8)].astNode), (yyvsp[(6) - (8)].astNode), (yyvsp[(8) - (8)].astNode), MEMMGR));
  }
    break;

  case 399:
#line 3514 "../src/parser/XQParser.y"
    {
    if((yyvsp[(1) - (3)].astNode)->getType() == ASTNode::OPERATOR && ((XQOperator*)(yyvsp[(1) - (3)].astNode))->getOperatorName() == Or::name) {
      ((Or*)(yyvsp[(1) - (3)].astNode))->addArgument((yyvsp[(3) - (3)].astNode));
      (yyval.astNode) = (yyvsp[(1) - (3)].astNode);
    }
    else
      (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) Or(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR), MEMMGR));
  }
    break;

  case 401:
#line 3528 "../src/parser/XQParser.y"
    {
    if((yyvsp[(1) - (3)].astNode)->getType()==ASTNode::OPERATOR && ((XQOperator*)(yyvsp[(1) - (3)].astNode))->getOperatorName() == And::name) {
      ((And*)(yyvsp[(1) - (3)].astNode))->addArgument((yyvsp[(3) - (3)].astNode));
      (yyval.astNode) = (yyvsp[(1) - (3)].astNode);
    }
    else
      (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) And(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR), MEMMGR));
  }
    break;

  case 403:
#line 3547 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) GeneralComp(GeneralComp::EQUAL,packageArgs((yyvsp[(1) - (3)].astNode),(yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
  }
    break;

  case 404:
#line 3551 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) GeneralComp(GeneralComp::NOT_EQUAL,packageArgs((yyvsp[(1) - (3)].astNode),(yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
  }
    break;

  case 405:
#line 3554 "../src/parser/XQParser.y"
    { /* Careful! */ QP->_lexer->undoLessThan(); }
    break;

  case 406:
#line 3555 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (4)]), new (MEMMGR) GeneralComp(GeneralComp::LESS_THAN,packageArgs((yyvsp[(1) - (4)].astNode),(yyvsp[(4) - (4)].astNode), MEMMGR),MEMMGR));
  }
    break;

  case 407:
#line 3559 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) GeneralComp(GeneralComp::LESS_THAN_EQUAL,packageArgs((yyvsp[(1) - (3)].astNode),(yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
  }
    break;

  case 408:
#line 3563 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) GeneralComp(GeneralComp::GREATER_THAN,packageArgs((yyvsp[(1) - (3)].astNode),(yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
  }
    break;

  case 409:
#line 3567 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) GeneralComp(GeneralComp::GREATER_THAN_EQUAL,packageArgs((yyvsp[(1) - (3)].astNode),(yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
  }
    break;

  case 410:
#line 3571 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) Equals(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
  }
    break;

  case 411:
#line 3575 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) NotEquals(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
  }
    break;

  case 412:
#line 3579 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) LessThan(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
  }
    break;

  case 413:
#line 3583 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) LessThanEqual(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
  }
    break;

  case 414:
#line 3587 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) GreaterThan(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
  }
    break;

  case 415:
#line 3591 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) GreaterThanEqual(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
  }
    break;

  case 416:
#line 3595 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) NodeComparison(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR), MEMMGR));
  }
    break;

  case 417:
#line 3599 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) OrderComparison(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR), true, MEMMGR));
  }
    break;

  case 418:
#line 3603 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) OrderComparison(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR), false,MEMMGR));
  }
    break;

  case 420:
#line 3612 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (4)]), new (MEMMGR) FTContains((yyvsp[(1) - (4)].astNode), (yyvsp[(4) - (4)].ftselection), NULL, MEMMGR));
  }
    break;

  case 421:
#line 3616 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (5)]), new (MEMMGR) FTContains((yyvsp[(1) - (5)].astNode), (yyvsp[(4) - (5)].ftselection), (yyvsp[(5) - (5)].astNode), MEMMGR));
  }
    break;

  case 423:
#line 3625 "../src/parser/XQParser.y"
    {
    VectorOfASTNodes *args = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    args->push_back((yyvsp[(1) - (3)].astNode));
    args->push_back((yyvsp[(3) - (3)].astNode));
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) XQFunctionCall(0, BuiltInModules::core.uri, MEMMGR->getPooledString("to"), args, MEMMGR));
  }
    break;

  case 425:
#line 3637 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) Plus(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
  }
    break;

  case 426:
#line 3641 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) Minus(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
  }
    break;

  case 428:
#line 3650 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) Multiply(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
  }
    break;

  case 429:
#line 3654 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) Divide(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
  }
    break;

  case 430:
#line 3658 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) IntegerDivide(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
  }
    break;

  case 431:
#line 3662 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) Mod(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
  }
    break;

  case 433:
#line 3671 "../src/parser/XQParser.y"
    {
      (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) Union(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
    }
    break;

  case 434:
#line 3675 "../src/parser/XQParser.y"
    {
      (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) Union(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
    }
    break;

  case 436:
#line 3684 "../src/parser/XQParser.y"
    {
      (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) Intersect(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
    }
    break;

  case 437:
#line 3688 "../src/parser/XQParser.y"
    {
      (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) Except(packageArgs((yyvsp[(1) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR),MEMMGR));
    }
    break;

  case 439:
#line 3697 "../src/parser/XQParser.y"
    {
    ASTNode *falseExpr =
      WRAP((yylsp[(2) - (4)]), new (MEMMGR) XQFunctionCall(0, XQFunction::XMLChFunctionURI,
                                           MEMMGR->getPooledString("false"), 0, MEMMGR));
    XQTypeswitch::Case *defcase =
      WRAP((yylsp[(1) - (4)]), new (MEMMGR) XQTypeswitch::Case(NULL, NULL, falseExpr));

    ASTNode *trueExpr =
      WRAP((yylsp[(2) - (4)]), new (MEMMGR) XQFunctionCall(0, XQFunction::XMLChFunctionURI,
                                           MEMMGR->getPooledString("true"), 0, MEMMGR));
    XQTypeswitch::Cases *cases = new (MEMMGR)
      XQTypeswitch::Cases(XQillaAllocator<XQTypeswitch::Case*>(MEMMGR));
    cases->push_back(WRAP((yylsp[(2) - (4)]), new (MEMMGR) XQTypeswitch::Case(NULL, (yyvsp[(4) - (4)].sequenceType), trueExpr)));

    (yyval.astNode) = WRAP((yylsp[(2) - (4)]), new (MEMMGR) XQTypeswitch((yyvsp[(1) - (4)].astNode), cases, defcase, MEMMGR));
  }
    break;

  case 441:
#line 3719 "../src/parser/XQParser.y"
    {
    XQTreatAs* treatAs = new (MEMMGR) XQTreatAs((yyvsp[(1) - (4)].astNode),(yyvsp[(4) - (4)].sequenceType),MEMMGR, XQTreatAs::err_XPDY0050);
    (yyval.astNode) = WRAP((yylsp[(2) - (4)]), treatAs);
  }
    break;

  case 443:
#line 3729 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (4)]), new (MEMMGR) XQCastableAs((yyvsp[(1) - (4)].astNode),(yyvsp[(4) - (4)].sequenceType),MEMMGR));
  }
    break;

  case 445:
#line 3738 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (4)]), new (MEMMGR) XQCastAs((yyvsp[(1) - (4)].astNode),(yyvsp[(4) - (4)].sequenceType),MEMMGR));
  }
    break;

  case 447:
#line 3747 "../src/parser/XQParser.y"
    {
      VectorOfASTNodes args(XQillaAllocator<ASTNode*>(MEMMGR));
      args.push_back((yyvsp[(2) - (2)].astNode));
      (yyval.astNode) = WRAP((yylsp[(1) - (2)]), new (MEMMGR) UnaryMinus(/*positive*/false, args, MEMMGR));
    }
    break;

  case 448:
#line 3753 "../src/parser/XQParser.y"
    {
      VectorOfASTNodes args(XQillaAllocator<ASTNode*>(MEMMGR));
      args.push_back((yyvsp[(2) - (2)].astNode));
      (yyval.astNode) = WRAP((yylsp[(1) - (2)]), new (MEMMGR) UnaryMinus(/*positive*/true, args, MEMMGR));
    }
    break;

  case 453:
#line 3774 "../src/parser/XQParser.y"
    {
      (yyval.astNode) = WRAP((yylsp[(1) - (4)]), new (MEMMGR) XQValidate((yyvsp[(3) - (4)].astNode),DocumentCache::VALIDATION_STRICT,MEMMGR));
    }
    break;

  case 454:
#line 3778 "../src/parser/XQParser.y"
    {
      (yyval.astNode) = WRAP((yylsp[(1) - (5)]), new (MEMMGR) XQValidate((yyvsp[(4) - (5)].astNode),DocumentCache::VALIDATION_LAX,MEMMGR));
    }
    break;

  case 455:
#line 3782 "../src/parser/XQParser.y"
    {
      (yyval.astNode) = WRAP((yylsp[(1) - (5)]), new (MEMMGR) XQValidate((yyvsp[(4) - (5)].astNode),DocumentCache::VALIDATION_STRICT,MEMMGR));
    }
    break;

  case 456:
#line 3790 "../src/parser/XQParser.y"
    {
    REJECT_NOT_XQUERY(ExtensionExpr, (yylsp[(1) - (3)]));

    // we don't support any pragma
    yyerror((yylsp[(1) - (3)]), "This pragma is not recognized, and no alternative expression is specified [err:XQST0079]");
  }
    break;

  case 457:
#line 3797 "../src/parser/XQParser.y"
    {
    REJECT_NOT_XQUERY(ExtensionExpr, (yylsp[(1) - (4)]));

    // we don't support any pragma
    (yyval.astNode) = (yyvsp[(3) - (4)].astNode);
  }
    break;

  case 460:
#line 3810 "../src/parser/XQParser.y"
    {
        // validate the QName
        QualifiedName qName((yyvsp[(3) - (4)].str));
        const XMLCh* prefix=qName.getPrefix();
        if(prefix == NULL || *prefix == 0)
          yyerror((yylsp[(3) - (4)]), "The pragma name must have a prefix [err:XPST0081]");

        try {
          LOCATION((yylsp[(3) - (4)]), loc);
          CONTEXT->getUriBoundToPrefix(prefix, &loc);
        }
        catch(NamespaceLookupException&) {
          yyerror((yylsp[(3) - (4)]), "The pragma name is using an undefined namespace prefix [err:XPST0081]");
        }
      }
    break;

  case 462:
#line 3831 "../src/parser/XQParser.y"
    {
    (yyval.str) = (yyvsp[(2) - (2)].str);
  }
    break;

  case 464:
#line 3840 "../src/parser/XQParser.y"
    {
    XQNav *nav = GET_NAVIGATION((yylsp[(1) - (2)]), (yyvsp[(1) - (2)].astNode));
    nav->addStep((yyvsp[(2) - (2)].astNode));
    (yyval.astNode) = nav;
  }
    break;

  case 465:
#line 3846 "../src/parser/XQParser.y"
    {
    XQNav *nav = WRAP((yylsp[(1) - (2)]), new (MEMMGR) XQNav(MEMMGR));

    VectorOfASTNodes args(XQillaAllocator<ASTNode*>(MEMMGR));
    FunctionRoot *root = WRAP((yylsp[(1) - (2)]), new (MEMMGR) FunctionRoot(args, MEMMGR));

    SequenceType *documentNode = WRAP((yylsp[(1) - (2)]), new (MEMMGR)
      SequenceType(new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_DOCUMENT)));

    nav->addStep(WRAP((yylsp[(1) - (2)]), new (MEMMGR) XQTreatAs(root, documentNode, MEMMGR)));

    NodeTest *step = new (MEMMGR) NodeTest();
    step->setTypeWildcard();
    step->setNameWildcard();
    step->setNamespaceWildcard();
    nav->addStep(WRAP((yylsp[(1) - (2)]), new (MEMMGR) XQStep(XQStep::DESCENDANT_OR_SELF, step, MEMMGR)));

    nav->addStep((yyvsp[(2) - (2)].astNode));
    (yyval.astNode) = nav;
  }
    break;

  case 467:
#line 3871 "../src/parser/XQParser.y"
    {
    VectorOfASTNodes args(XQillaAllocator<ASTNode*>(MEMMGR));
    FunctionRoot *root = WRAP((yylsp[(1) - (1)]), new (MEMMGR) FunctionRoot(args, MEMMGR));

    SequenceType *documentNode = WRAP((yylsp[(1) - (1)]), new (MEMMGR)
      SequenceType(new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_DOCUMENT)));

    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQTreatAs(root, documentNode, MEMMGR));
  }
    break;

  case 468:
#line 3885 "../src/parser/XQParser.y"
    {
    XQNav *nav = GET_NAVIGATION((yylsp[(1) - (3)]), (yyvsp[(1) - (3)].astNode));
    nav->addStep((yyvsp[(3) - (3)].astNode));
    (yyval.astNode) = nav;
  }
    break;

  case 469:
#line 3891 "../src/parser/XQParser.y"
    {
    XQNav *nav = GET_NAVIGATION((yylsp[(1) - (3)]), (yyvsp[(1) - (3)].astNode));

    NodeTest *step = new (MEMMGR) NodeTest();
    step->setTypeWildcard();
    step->setNameWildcard();
    step->setNamespaceWildcard();
    nav->addStep(WRAP((yylsp[(2) - (3)]), new (MEMMGR) XQStep(XQStep::DESCENDANT_OR_SELF, step, MEMMGR)));
    nav->addStep((yyvsp[(3) - (3)].astNode));

    (yyval.astNode) = nav;
  }
    break;

  case 474:
#line 3913 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQDocumentOrder((yyvsp[(1) - (1)].astNode), MEMMGR));
  }
    break;

  case 476:
#line 3923 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (4)]), new (MEMMGR) XQPredicate((yyvsp[(1) - (4)].astNode), (yyvsp[(3) - (4)].astNode), MEMMGR));
  }
    break;

  case 478:
#line 3930 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (4)]), new (MEMMGR) XQPredicate((yyvsp[(1) - (4)].astNode), (yyvsp[(3) - (4)].astNode), /*reverse*/true, MEMMGR));
  }
    break;

  case 479:
#line 3939 "../src/parser/XQParser.y"
    {
    if(!(yyvsp[(2) - (2)].nodeTest)->isNodeTypeSet()) {
      switch((yyvsp[(1) - (2)].axis)) {
      case XQStep::NAMESPACE: (yyvsp[(2) - (2)].nodeTest)->setNodeType(Node::namespace_string); break;
      case XQStep::ATTRIBUTE: (yyvsp[(2) - (2)].nodeTest)->setNodeType(Node::attribute_string); break;
      default: (yyvsp[(2) - (2)].nodeTest)->setNodeType(Node::element_string); break;
      }
    }

    (yyval.astNode) = WRAP((yylsp[(1) - (2)]), new (MEMMGR) XQStep((yyvsp[(1) - (2)].axis),(yyvsp[(2) - (2)].nodeTest),MEMMGR));
  }
    break;

  case 481:
#line 3962 "../src/parser/XQParser.y"
    {
    (yyval.axis) = XQStep::CHILD;
  }
    break;

  case 482:
#line 3966 "../src/parser/XQParser.y"
    {
    (yyval.axis) = XQStep::DESCENDANT;
  }
    break;

  case 483:
#line 3970 "../src/parser/XQParser.y"
    {
    (yyval.axis) = XQStep::ATTRIBUTE;
  }
    break;

  case 484:
#line 3974 "../src/parser/XQParser.y"
    {
    (yyval.axis) = XQStep::SELF;
  }
    break;

  case 485:
#line 3978 "../src/parser/XQParser.y"
    {
    (yyval.axis) = XQStep::DESCENDANT_OR_SELF;
  }
    break;

  case 486:
#line 3982 "../src/parser/XQParser.y"
    {
    (yyval.axis) = XQStep::FOLLOWING_SIBLING;
  }
    break;

  case 487:
#line 3986 "../src/parser/XQParser.y"
    {
    (yyval.axis) = XQStep::FOLLOWING;
  }
    break;

  case 488:
#line 3990 "../src/parser/XQParser.y"
    {
    (yyval.axis) = XQStep::NAMESPACE;
  }
    break;

  case 489:
#line 3998 "../src/parser/XQParser.y"
    {
    if(!(yyvsp[(2) - (2)].nodeTest)->isNodeTypeSet()) {
      (yyvsp[(2) - (2)].nodeTest)->setNodeType(Node::attribute_string);
    }

    (yyval.astNode) = WRAP((yylsp[(1) - (2)]), new (MEMMGR) XQStep(XQStep::ATTRIBUTE, (yyvsp[(2) - (2)].nodeTest), MEMMGR));
  }
    break;

  case 490:
#line 4006 "../src/parser/XQParser.y"
    {
    XQStep::Axis axis = XQStep::CHILD;
    SequenceType::ItemType *itemtype = (yyvsp[(1) - (1)].nodeTest)->getItemType();
    if(itemtype != 0 && itemtype->getItemTestType() == SequenceType::ItemType::TEST_ATTRIBUTE) {
      axis = XQStep::ATTRIBUTE;
    }
    else if(!(yyvsp[(1) - (1)].nodeTest)->isNodeTypeSet()) {
      (yyvsp[(1) - (1)].nodeTest)->setNodeType(Node::element_string);
    }

    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQStep(axis, (yyvsp[(1) - (1)].nodeTest), MEMMGR));
  }
    break;

  case 491:
#line 4023 "../src/parser/XQParser.y"
    {
    if(!(yyvsp[(2) - (2)].nodeTest)->isNodeTypeSet()) {
      (yyvsp[(2) - (2)].nodeTest)->setNodeType(Node::element_string);
    }

    (yyval.astNode) = WRAP((yylsp[(1) - (2)]), new (MEMMGR) XQStep((yyvsp[(1) - (2)].axis), (yyvsp[(2) - (2)].nodeTest), MEMMGR));
  }
    break;

  case 493:
#line 4040 "../src/parser/XQParser.y"
    {
    (yyval.axis) = XQStep::PARENT;
  }
    break;

  case 494:
#line 4044 "../src/parser/XQParser.y"
    {
    (yyval.axis) = XQStep::ANCESTOR;
  }
    break;

  case 495:
#line 4048 "../src/parser/XQParser.y"
    {
    (yyval.axis) = XQStep::PRECEDING_SIBLING;
  }
    break;

  case 496:
#line 4052 "../src/parser/XQParser.y"
    {
    (yyval.axis) = XQStep::PRECEDING;
  }
    break;

  case 497:
#line 4056 "../src/parser/XQParser.y"
    {
    (yyval.axis) = XQStep::ANCESTOR_OR_SELF;
  }
    break;

  case 498:
#line 4064 "../src/parser/XQParser.y"
    {
    NodeTest *step = new (MEMMGR) NodeTest();
    step->setNameWildcard();
    step->setNamespaceWildcard();
    step->setTypeWildcard();
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQStep(XQStep::PARENT, step, MEMMGR));
  }
    break;

  case 499:
#line 4076 "../src/parser/XQParser.y"
    {
    (yyval.nodeTest) = new (MEMMGR) NodeTest();
    (yyval.nodeTest)->setItemType((yyvsp[(1) - (1)].itemType));
  }
    break;

  case 501:
#line 4086 "../src/parser/XQParser.y"
    {
    NodeTest *step = new (MEMMGR) NodeTest();
    step->setNodePrefix((yyvsp[(1) - (1)].qName)->getPrefix());
    step->setNodeName((yyvsp[(1) - (1)].qName)->getName());
    (yyval.nodeTest) = step;
  }
    break;

  case 503:
#line 4098 "../src/parser/XQParser.y"
    {
    NodeTest *step = new (MEMMGR) NodeTest();
    step->setNameWildcard();
    step->setNamespaceWildcard();
    (yyval.nodeTest) = step;
  }
    break;

  case 504:
#line 4105 "../src/parser/XQParser.y"
    {
    NodeTest *step = new (MEMMGR) NodeTest();
    step->setNodePrefix((yyvsp[(1) - (1)].str));
    step->setNameWildcard();
    (yyval.nodeTest) = step;
  }
    break;

  case 505:
#line 4112 "../src/parser/XQParser.y"
    {
    NodeTest *step = new (MEMMGR) NodeTest();
    step->setNodeName((yyvsp[(1) - (1)].str));
    step->setNamespaceWildcard();
    (yyval.nodeTest) = step;
  }
    break;

  case 507:
#line 4125 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(2) - (4)]), new (MEMMGR) XQPredicate((yyvsp[(1) - (4)].astNode), (yyvsp[(3) - (4)].astNode), MEMMGR));
  }
    break;

  case 523:
#line 4154 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (2)]), new (MEMMGR) XQVariable((yyvsp[(2) - (2)].str), MEMMGR));
  }
    break;

  case 525:
#line 4165 "../src/parser/XQParser.y"
    { 
    (yyval.astNode) = (yyvsp[(2) - (3)].astNode);
  }
    break;

  case 526:
#line 4169 "../src/parser/XQParser.y"
    { 
    (yyval.astNode) = WRAP((yylsp[(1) - (2)]), new (MEMMGR) XQSequence(MEMMGR));
  }
    break;

  case 527:
#line 4177 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQContextItem(MEMMGR));
  }
    break;

  case 528:
#line 4185 "../src/parser/XQParser.y"
    {
    REJECT_NOT_XQUERY(OrderedExpr, (yylsp[(1) - (4)]));

    (yyval.astNode) = WRAP((yylsp[(1) - (4)]), new (MEMMGR) XQOrderingChange(StaticContext::ORDERING_ORDERED, (yyvsp[(3) - (4)].astNode), MEMMGR));
  }
    break;

  case 529:
#line 4195 "../src/parser/XQParser.y"
    {
    REJECT_NOT_XQUERY(UnorderedExpr, (yylsp[(1) - (4)]));

    (yyval.astNode) = WRAP((yylsp[(1) - (4)]), new (MEMMGR) XQOrderingChange(StaticContext::ORDERING_UNORDERED, (yyvsp[(3) - (4)].astNode), MEMMGR));
  }
    break;

  case 530:
#line 4205 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQFunctionCall((yyvsp[(1) - (3)].str), NULL, MEMMGR));
  }
    break;

  case 531:
#line 4209 "../src/parser/XQParser.y"
    {
    bool partial = false;
    VectorOfASTNodes::iterator i;
    for(i = (yyvsp[(3) - (4)].itemList)->begin(); i != (yyvsp[(3) - (4)].itemList)->end(); ++i) {
      if(*i == 0) {
        partial = true;
        break;
      }
    }

    if(partial) {
      // This is a partial function application
      XQFunctionRef *ref = WRAP((yylsp[(1) - (4)]), new (MEMMGR) XQFunctionRef((yyvsp[(1) - (4)].str), (yyvsp[(3) - (4)].itemList)->size(), MEMMGR));
      (yyval.astNode) = WRAP((yylsp[(2) - (4)]), new (MEMMGR) XQPartialApply(ref, (yyvsp[(3) - (4)].itemList), MEMMGR));
    }
    else {
      (yyval.astNode) = WRAP((yylsp[(1) - (4)]), new (MEMMGR) XQFunctionCall((yyvsp[(1) - (4)].str), (yyvsp[(3) - (4)].itemList), MEMMGR));
    }
  }
    break;

  case 532:
#line 4232 "../src/parser/XQParser.y"
    {
    (yyvsp[(1) - (3)].itemList)->push_back((yyvsp[(3) - (3)].astNode));
    (yyval.itemList) = (yyvsp[(1) - (3)].itemList);
  }
    break;

  case 533:
#line 4237 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    (yyval.itemList)->push_back((yyvsp[(1) - (1)].astNode));
  }
    break;

  case 535:
#line 4248 "../src/parser/XQParser.y"
    {
    REJECT_NOT_VERSION3(Argument, (yylsp[(1) - (1)]));
    (yyval.astNode) = 0;
  }
    break;

  case 536:
#line 4257 "../src/parser/XQParser.y"
    {
    REJECT_NOT_XQUERY(Constructor, (yylsp[(1) - (1)]));
  }
    break;

  case 537:
#line 4261 "../src/parser/XQParser.y"
    {
    REJECT_NOT_XQUERY(Constructor, (yylsp[(1) - (1)]));
  }
    break;

  case 541:
#line 4274 "../src/parser/XQParser.y"
    { 
    VectorOfASTNodes* content = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    ASTNode *name = WRAP((yylsp[(2) - (5)]), new (MEMMGR) XQDirectName((yyvsp[(2) - (5)].str), /*useDefaultNamespace*/true, MEMMGR));
    (yyval.astNode) = WRAP((yylsp[(1) - (5)]), new (MEMMGR) XQElementConstructor(name, (yyvsp[(3) - (5)].itemList), content, MEMMGR));
  }
    break;

  case 542:
#line 4280 "../src/parser/XQParser.y"
    { 
    if(!XPath2Utils::equals((yyvsp[(2) - (10)].str), (yyvsp[(8) - (10)].str)))
      yyerror((yylsp[(7) - (10)]), "Close tag does not match open tag");
    ASTNode *name = WRAP((yylsp[(2) - (10)]), new (MEMMGR) XQDirectName((yyvsp[(2) - (10)].str), /*useDefaultNamespace*/true, MEMMGR));
    (yyval.astNode) = WRAP((yylsp[(1) - (10)]), new (MEMMGR) XQElementConstructor(name, (yyvsp[(3) - (10)].itemList), (yyvsp[(6) - (10)].itemList), MEMMGR));
  }
    break;

  case 545:
#line 4293 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
  }
    break;

  case 546:
#line 4297 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = (yyvsp[(1) - (7)].itemList);

    ASTNode *name = WRAP((yylsp[(3) - (7)]), new (MEMMGR) XQDirectName((yyvsp[(3) - (7)].str), /*useDefaultNamespace*/false, MEMMGR));
    ASTNode *attrItem = WRAP((yylsp[(3) - (7)]), new (MEMMGR) XQAttributeConstructor(name, (yyvsp[(7) - (7)].itemList),MEMMGR));

    (yyval.itemList)->push_back(attrItem);
  }
    break;

  case 547:
#line 4306 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = (yyvsp[(1) - (7)].itemList);

    ASTNode *name = WRAP((yylsp[(3) - (7)]), new (MEMMGR) XQDirectName((yyvsp[(3) - (7)].str), /*useDefaultNamespace*/false, MEMMGR));
    ASTNode *attrItem = WRAP((yylsp[(3) - (7)]), new (MEMMGR) XQAttributeConstructor(name, (yyvsp[(7) - (7)].itemList),MEMMGR));

    (yyval.itemList)->insert((yyval.itemList)->begin(), attrItem);
  }
    break;

  case 550:
#line 4322 "../src/parser/XQParser.y"
    { 
    (yyval.itemList) = (yyvsp[(2) - (3)].itemList);
  }
    break;

  case 551:
#line 4326 "../src/parser/XQParser.y"
    { 
    (yyval.itemList) = (yyvsp[(2) - (3)].itemList);
  }
    break;

  case 552:
#line 4333 "../src/parser/XQParser.y"
    { 
    (yyval.itemList) = (yyvsp[(2) - (3)].itemList);
  }
    break;

  case 553:
#line 4337 "../src/parser/XQParser.y"
    { 
    (yyval.itemList) = (yyvsp[(2) - (3)].itemList);
  }
    break;

  case 554:
#line 4347 "../src/parser/XQParser.y"
    { 
    (yyval.itemList) = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
  }
    break;

  case 555:
#line 4351 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = (yyvsp[(1) - (2)].itemList);
    (yyval.itemList)->push_back((yyvsp[(2) - (2)].astNode));
  }
    break;

  case 556:
#line 4356 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = (yyvsp[(1) - (2)].itemList);
    (yyval.itemList)->push_back(WRAP((yylsp[(2) - (2)]), new (MEMMGR) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               SchemaSymbols::fgDT_STRING,
                               (yyvsp[(2) - (2)].str), AnyAtomicType::STRING, MEMMGR)));
  }
    break;

  case 557:
#line 4366 "../src/parser/XQParser.y"
    { 
    (yyval.itemList) = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
  }
    break;

  case 558:
#line 4370 "../src/parser/XQParser.y"
    {
    yyerror((yylsp[(2) - (2)]), "Namespace URI of a namespace declaration must be a literal [err:XQST0022]");
  }
    break;

  case 559:
#line 4374 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = (yyvsp[(1) - (2)].itemList);
    (yyval.itemList)->push_back(WRAP((yylsp[(2) - (2)]), new (MEMMGR) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               SchemaSymbols::fgDT_STRING,
                               (yyvsp[(2) - (2)].str), AnyAtomicType::STRING, MEMMGR)));
  }
    break;

  case 560:
#line 4387 "../src/parser/XQParser.y"
    { 
    (yyval.itemList) = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
  }
    break;

  case 561:
#line 4391 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = (yyvsp[(1) - (2)].itemList);
    (yyval.itemList)->push_back((yyvsp[(2) - (2)].astNode));
  }
    break;

  case 562:
#line 4396 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = (yyvsp[(1) - (2)].itemList);
    (yyval.itemList)->push_back(WRAP((yylsp[(2) - (2)]), new (MEMMGR) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               SchemaSymbols::fgDT_STRING,
                               (yyvsp[(2) - (2)].str), AnyAtomicType::STRING, MEMMGR)));
  }
    break;

  case 563:
#line 4406 "../src/parser/XQParser.y"
    { 
    (yyval.itemList) = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
  }
    break;

  case 564:
#line 4410 "../src/parser/XQParser.y"
    {
    yyerror((yylsp[(2) - (2)]), "Namespace URI of a namespace declaration must be a literal [err:XQST0022]");
  }
    break;

  case 565:
#line 4414 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = (yyvsp[(1) - (2)].itemList);
    (yyval.itemList)->push_back(WRAP((yylsp[(2) - (2)]), new (MEMMGR) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               SchemaSymbols::fgDT_STRING,
                               (yyvsp[(2) - (2)].str), AnyAtomicType::STRING, MEMMGR)));
  }
    break;

  case 566:
#line 4431 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR)); 
  }
    break;

  case 567:
#line 4435 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = (yyvsp[(1) - (2)].itemList);
    (yyval.itemList)->push_back((yyvsp[(2) - (2)].astNode));
  }
    break;

  case 568:
#line 4440 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = (yyvsp[(1) - (2)].itemList);
    (yyval.itemList)->push_back((yyvsp[(2) - (2)].astNode));
  }
    break;

  case 569:
#line 4445 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = (yyvsp[(1) - (2)].itemList);
    (yyval.itemList)->push_back(WRAP((yylsp[(2) - (2)]), new (MEMMGR) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               SchemaSymbols::fgDT_STRING,
                               (yyvsp[(2) - (2)].str), AnyAtomicType::STRING, MEMMGR)));
  }
    break;

  case 570:
#line 4452 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = (yyvsp[(1) - (2)].itemList);
    if(CONTEXT->getPreserveBoundarySpace()) {
      (yyval.itemList)->push_back(WRAP((yylsp[(2) - (2)]), new (MEMMGR) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                 SchemaSymbols::fgDT_STRING,
                                 (yyvsp[(2) - (2)].str), AnyAtomicType::STRING, MEMMGR)));
    }
  }
    break;

  case 571:
#line 4465 "../src/parser/XQParser.y"
    {
    ASTNode *value = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQLiteral(
                  SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                  SchemaSymbols::fgDT_STRING,
                  (yyvsp[(2) - (3)].str), AnyAtomicType::STRING,
                  MEMMGR));
    (yyval.astNode) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQCommentConstructor(value, MEMMGR));
  }
    break;

  case 572:
#line 4478 "../src/parser/XQParser.y"
    {
    (yyval.str) = (XMLCh*)XMLUni::fgZeroLenString;
  }
    break;

  case 574:
#line 4487 "../src/parser/XQParser.y"
    {
    ASTNode *value = WRAP((yylsp[(3) - (3)]), new (MEMMGR) XQLiteral(
                  SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                  SchemaSymbols::fgDT_STRING,
                  (yyvsp[(3) - (3)].str), AnyAtomicType::STRING,
                  MEMMGR));
    (yyval.astNode) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQPIConstructor(
                    WRAP((yylsp[(2) - (3)]), new (MEMMGR) XQLiteral(
                  SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                  SchemaSymbols::fgDT_STRING,
                  (yyvsp[(2) - (3)].str), AnyAtomicType::STRING,
                  MEMMGR)), 
                  value, MEMMGR));
  }
    break;

  case 576:
#line 4507 "../src/parser/XQParser.y"
    {
    (yyval.str) = (yyvsp[(2) - (2)].str);
  }
    break;

  case 584:
#line 4532 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (4)]), new (MEMMGR) XQDocumentConstructor((yyvsp[(3) - (4)].astNode), MEMMGR));
  }
    break;

  case 585:
#line 4540 "../src/parser/XQParser.y"
    {
    VectorOfASTNodes* empty = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    (yyval.astNode) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQElementConstructor((yyvsp[(2) - (3)].astNode), empty, (yyvsp[(3) - (3)].itemList), MEMMGR));
  }
    break;

  case 586:
#line 4548 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQDirectName((yyvsp[(1) - (1)].str), /*useDefaultNamespace*/true, MEMMGR));
  }
    break;

  case 587:
#line 4552 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(2) - (3)].astNode);
  }
    break;

  case 588:
#line 4560 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
  }
    break;

  case 589:
#line 4564 "../src/parser/XQParser.y"
    {
    (yyval.itemList) = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    (yyval.itemList)->push_back((yyvsp[(2) - (3)].astNode));
  }
    break;

  case 590:
#line 4573 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQAttributeConstructor((yyvsp[(2) - (3)].astNode), (yyvsp[(3) - (3)].itemList), MEMMGR));
  }
    break;

  case 591:
#line 4580 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQDirectName((yyvsp[(1) - (1)].str), /*useDefaultNamespace*/false, MEMMGR));
  }
    break;

  case 592:
#line 4584 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(2) - (3)].astNode);
  }
    break;

  case 593:
#line 4594 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQNamespaceConstructor((yyvsp[(2) - (3)].astNode), (yyvsp[(3) - (3)].itemList), MEMMGR));
  }
    break;

  case 594:
#line 4602 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (4)]), new (MEMMGR) XQTextConstructor((yyvsp[(3) - (4)].astNode), MEMMGR));
  }
    break;

  case 595:
#line 4610 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (4)]), new (MEMMGR) XQCommentConstructor((yyvsp[(3) - (4)].astNode), MEMMGR));
  }
    break;

  case 596:
#line 4618 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQPIConstructor((yyvsp[(2) - (3)].astNode), (yyvsp[(3) - (3)].astNode), MEMMGR));
  }
    break;

  case 597:
#line 4625 "../src/parser/XQParser.y"
    {
    // Check for a colon
    for(XMLCh *tmp = (yyvsp[(1) - (1)].str); *tmp; ++tmp)
      if(*tmp == ':') yyerror((yylsp[(1) - (1)]), "Expecting an NCName, found a QName");

    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               SchemaSymbols::fgDT_STRING,
                               (yyvsp[(1) - (1)].str), AnyAtomicType::STRING, MEMMGR));
  }
    break;

  case 598:
#line 4635 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(2) - (3)].astNode);
  }
    break;

  case 599:
#line 4642 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (2)]), new (MEMMGR) XQSequence(MEMMGR));
  }
    break;

  case 600:
#line 4646 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(2) - (3)].astNode);
  }
    break;

  case 601:
#line 4654 "../src/parser/XQParser.y"
    {
    SequenceType* seq = WRAP((yylsp[(1) - (2)]), new (MEMMGR) SequenceType());
    seq->setItemType(new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ATOMIC_TYPE, NULL, (yyvsp[(1) - (2)].qName)));
    seq->setOccurrence((yyvsp[(2) - (2)].occurrence));
    (yyval.sequenceType) = seq;
  }
    break;

  case 602:
#line 4664 "../src/parser/XQParser.y"
    {
    (yyval.occurrence) = SequenceType::EXACTLY_ONE;
  }
    break;

  case 603:
#line 4668 "../src/parser/XQParser.y"
    {
    (yyval.occurrence) = SequenceType::QUESTION_MARK;
  }
    break;

  case 604:
#line 4676 "../src/parser/XQParser.y"
    {
    (yyval.sequenceType) = WRAP((yyloc), new (MEMMGR) SequenceType(new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ANYTHING), SequenceType::STAR));
  }
    break;

  case 605:
#line 4680 "../src/parser/XQParser.y"
    {
    REJECT_NOT_XQUERY(TypeDeclaration, (yylsp[(1) - (2)]));

    (yyval.sequenceType) = (yyvsp[(2) - (2)].sequenceType);
  }
    break;

  case 606:
#line 4690 "../src/parser/XQParser.y"
    {
    SequenceType* seq = WRAP((yylsp[(1) - (2)]), new (MEMMGR) SequenceType());
    seq->setItemType((yyvsp[(1) - (2)].itemType));
    seq->setOccurrence((yyvsp[(2) - (2)].occurrence));
    (yyval.sequenceType) = seq;
  }
    break;

  case 607:
#line 4697 "../src/parser/XQParser.y"
    { 
    (yyval.sequenceType) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) SequenceType()); 
  }
    break;

  case 608:
#line 4706 "../src/parser/XQParser.y"
    { (yyval.occurrence) = SequenceType::EXACTLY_ONE; }
    break;

  case 609:
#line 4708 "../src/parser/XQParser.y"
    { (yyval.occurrence) = SequenceType::STAR; }
    break;

  case 610:
#line 4710 "../src/parser/XQParser.y"
    { (yyval.occurrence) = SequenceType::PLUS; }
    break;

  case 611:
#line 4712 "../src/parser/XQParser.y"
    { (yyval.occurrence) = SequenceType::QUESTION_MARK; }
    break;

  case 612:
#line 4718 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ATOMIC_TYPE, NULL, (yyvsp[(1) - (1)].qName));
  }
    break;

  case 613:
#line 4722 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ANYTHING);
  }
    break;

  case 628:
#line 4758 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_NODE);
  }
    break;

  case 629:
#line 4766 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_DOCUMENT);
  }
    break;

  case 630:
#line 4770 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = (yyvsp[(3) - (4)].itemType);
    (yyval.itemType)->setItemTestType(SequenceType::ItemType::TEST_DOCUMENT);
  }
    break;

  case 631:
#line 4775 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = (yyvsp[(3) - (4)].itemType);
    (yyval.itemType)->setItemTestType(SequenceType::ItemType::TEST_SCHEMA_DOCUMENT);
  }
    break;

  case 632:
#line 4784 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_TEXT);
  }
    break;

  case 633:
#line 4792 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_COMMENT);
  }
    break;

  case 634:
#line 4800 "../src/parser/XQParser.y"
    {
    REJECT_NOT_VERSION3(NamespaceNodeTest, (yylsp[(1) - (3)]));
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_NAMESPACE);
  }
    break;

  case 635:
#line 4809 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_PI);
  }
    break;

  case 636:
#line 4813 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_PI, new (MEMMGR) QualifiedName((yyvsp[(3) - (4)].str), MEMMGR));
  }
    break;

  case 637:
#line 4817 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_PI, new (MEMMGR) QualifiedName((yyvsp[(3) - (4)].str), MEMMGR));
  }
    break;

  case 638:
#line 4825 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ATTRIBUTE);
  }
    break;

  case 639:
#line 4829 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ATTRIBUTE,(yyvsp[(3) - (4)].qName));
  }
    break;

  case 640:
#line 4833 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ATTRIBUTE,(yyvsp[(3) - (6)].qName),(yyvsp[(5) - (6)].qName));
  }
    break;

  case 642:
#line 4842 "../src/parser/XQParser.y"
    {
    (yyval.qName) = NULL;
  }
    break;

  case 643:
#line 4850 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_SCHEMA_ATTRIBUTE,(yyvsp[(3) - (4)].qName));
  }
    break;

  case 645:
#line 4863 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ELEMENT);
  }
    break;

  case 646:
#line 4867 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ELEMENT,(yyvsp[(3) - (4)].qName));
  }
    break;

  case 647:
#line 4871 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ELEMENT,(yyvsp[(3) - (6)].qName),(yyvsp[(5) - (6)].qName));
  }
    break;

  case 648:
#line 4875 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ELEMENT,(yyvsp[(3) - (7)].qName),(yyvsp[(5) - (7)].qName));
    (yyval.itemType)->setAllowNilled(true);
  }
    break;

  case 650:
#line 4885 "../src/parser/XQParser.y"
    {
    (yyval.qName) = NULL;
  }
    break;

  case 651:
#line 4893 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_SCHEMA_ELEMENT,(yyvsp[(3) - (4)].qName));
  }
    break;

  case 656:
#line 4913 "../src/parser/XQParser.y"
    {
    // the string must be whitespace-normalized
    XMLString::collapseWS((yyvsp[(1) - (1)].str), MEMMGR);
    if((yyvsp[(1) - (1)].str) && *(yyvsp[(1) - (1)].str) && !XPath2Utils::isValidURI((yyvsp[(1) - (1)].str), MEMMGR))
      yyerror((yylsp[(1) - (1)]), "The URI literal is not valid [err:XQST0046]");
    (yyval.str) = (yyvsp[(1) - (1)].str);
  }
    break;

  case 657:
#line 4925 "../src/parser/XQParser.y"
    {
    // TBD weight
    (yyval.ftselection) = (yyvsp[(1) - (3)].ftselection);

    for(VectorOfFTOptions::iterator i = (yyvsp[(2) - (3)].ftoptionlist)->begin();
        i != (yyvsp[(2) - (3)].ftoptionlist)->end(); ++i) {
      (*i)->setArgument((yyval.ftselection));
      (yyval.ftselection) = *i;
    }
/*     delete $2; */
  }
    break;

  case 658:
#line 4940 "../src/parser/XQParser.y"
    {
}
    break;

  case 659:
#line 4943 "../src/parser/XQParser.y"
    {
}
    break;

  case 660:
#line 4949 "../src/parser/XQParser.y"
    {
  (yyval.ftoptionlist) = new (MEMMGR) VectorOfFTOptions(XQillaAllocator<FTOption*>(MEMMGR));
}
    break;

  case 661:
#line 4953 "../src/parser/XQParser.y"
    {
  if((yyvsp[(2) - (2)].ftoption) != NULL) (yyvsp[(1) - (2)].ftoptionlist)->push_back((yyvsp[(2) - (2)].ftoption));
  (yyval.ftoptionlist) = (yyvsp[(1) - (2)].ftoptionlist);
}
    break;

  case 662:
#line 4963 "../src/parser/XQParser.y"
    {
    if((yyvsp[(1) - (3)].ftselection)->getType() == FTSelection::OR) {
      FTOr *op = (FTOr*)(yyvsp[(1) - (3)].ftselection);
      op->addArg((yyvsp[(3) - (3)].ftselection));
      (yyval.ftselection) = op;
    }
    else {
      (yyval.ftselection) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) FTOr((yyvsp[(1) - (3)].ftselection), (yyvsp[(3) - (3)].ftselection), MEMMGR));
    }
  }
    break;

  case 664:
#line 4980 "../src/parser/XQParser.y"
    {
    if((yyvsp[(1) - (3)].ftselection)->getType() == FTSelection::AND) {
      FTAnd *op = (FTAnd*)(yyvsp[(1) - (3)].ftselection);
      op->addArg((yyvsp[(3) - (3)].ftselection));
      (yyval.ftselection) = op;
    }
    else {
      (yyval.ftselection) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) FTAnd((yyvsp[(1) - (3)].ftselection), (yyvsp[(3) - (3)].ftselection), MEMMGR));
    }
  }
    break;

  case 666:
#line 4996 "../src/parser/XQParser.y"
    {
    (yyval.ftselection) = WRAP((yylsp[(2) - (4)]), new (MEMMGR) FTMildnot((yyvsp[(1) - (4)].ftselection), (yyvsp[(4) - (4)].ftselection), MEMMGR));
  }
    break;

  case 668:
#line 5006 "../src/parser/XQParser.y"
    {
    (yyval.ftselection) = WRAP((yylsp[(1) - (2)]), new (MEMMGR) FTUnaryNot((yyvsp[(2) - (2)].ftselection), MEMMGR));
  }
    break;

  case 671:
#line 5016 "../src/parser/XQParser.y"
    {
  // TBD match options
  (yyval.ftselection) = (yyvsp[(1) - (2)].ftselection);
}
    break;

  case 672:
#line 5025 "../src/parser/XQParser.y"
    {
  // TBD FTTimes
  (yyval.ftselection) = (yyvsp[(1) - (2)].ftselection);
}
    break;

  case 673:
#line 5030 "../src/parser/XQParser.y"
    {
  (yyval.ftselection) = (yyvsp[(2) - (3)].ftselection);
}
    break;

  case 675:
#line 5040 "../src/parser/XQParser.y"
    {
    (yyval.ftselection) = WRAP((yylsp[(1) - (2)]), new (MEMMGR) FTWords((yyvsp[(1) - (2)].astNode), (yyvsp[(2) - (2)].ftanyalloption), MEMMGR));
  }
    break;

  case 676:
#line 5044 "../src/parser/XQParser.y"
    {
    (yyval.ftselection) = WRAP((yylsp[(2) - (4)]), new (MEMMGR) FTWords((yyvsp[(2) - (4)].astNode), (yyvsp[(4) - (4)].ftanyalloption), MEMMGR));
  }
    break;

  case 677:
#line 5052 "../src/parser/XQParser.y"
    {
  // we don't support any pragma
  yyerror((yylsp[(1) - (3)]), "This pragma is not recognized, and no alternative expression is specified [err:XQST0079]");
}
    break;

  case 678:
#line 5057 "../src/parser/XQParser.y"
    {
  // we don't support any pragma
  (yyval.ftselection) = (yyvsp[(3) - (4)].ftselection);
}
    break;

  case 679:
#line 5066 "../src/parser/XQParser.y"
    {
    (yyval.ftanyalloption) = FTWords::ANY;
  }
    break;

  case 680:
#line 5070 "../src/parser/XQParser.y"
    {
    (yyval.ftanyalloption) = FTWords::ANY;
  }
    break;

  case 681:
#line 5074 "../src/parser/XQParser.y"
    {
    (yyval.ftanyalloption) = FTWords::ANY_WORD;
  }
    break;

  case 682:
#line 5078 "../src/parser/XQParser.y"
    {
    (yyval.ftanyalloption) = FTWords::ALL;
  }
    break;

  case 683:
#line 5082 "../src/parser/XQParser.y"
    {
    (yyval.ftanyalloption) = FTWords::ALL_WORDS;
  }
    break;

  case 684:
#line 5086 "../src/parser/XQParser.y"
    {
    (yyval.ftanyalloption) = FTWords::PHRASE;
  }
    break;

  case 686:
#line 5096 "../src/parser/XQParser.y"
    {
  yyerror((yylsp[(1) - (3)]), "The FTTimes operator is not supported. [err:FTST0005]");
}
    break;

  case 687:
#line 5107 "../src/parser/XQParser.y"
    {
    (yyval.ftrange).type = FTRange::EXACTLY;
    (yyval.ftrange).arg1 = (yyvsp[(2) - (2)].astNode);
    (yyval.ftrange).arg2 = 0;
  }
    break;

  case 688:
#line 5113 "../src/parser/XQParser.y"
    {
    (yyval.ftrange).type = FTRange::AT_LEAST;
    (yyval.ftrange).arg1 = (yyvsp[(3) - (3)].astNode);
    (yyval.ftrange).arg2 = 0;
  }
    break;

  case 689:
#line 5119 "../src/parser/XQParser.y"
    {
    (yyval.ftrange).type = FTRange::AT_MOST;
    (yyval.ftrange).arg1 = (yyvsp[(3) - (3)].astNode);
    (yyval.ftrange).arg2 = 0;
  }
    break;

  case 690:
#line 5125 "../src/parser/XQParser.y"
    {
    (yyval.ftrange).type = FTRange::FROM_TO;
    (yyval.ftrange).arg1 = (yyvsp[(2) - (4)].astNode);
    (yyval.ftrange).arg2 = (yyvsp[(4) - (4)].astNode);
  }
    break;

  case 691:
#line 5140 "../src/parser/XQParser.y"
    {
    (yyval.ftoption) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) FTOrder(MEMMGR));
  }
    break;

  case 692:
#line 5144 "../src/parser/XQParser.y"
    {
    (yyval.ftoption) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) FTWindow((yyvsp[(2) - (3)].astNode), (yyvsp[(3) - (3)].ftunit), MEMMGR));
  }
    break;

  case 693:
#line 5148 "../src/parser/XQParser.y"
    {
    (yyval.ftoption) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) FTDistance((yyvsp[(2) - (3)].ftrange), (yyvsp[(3) - (3)].ftunit), MEMMGR));
  }
    break;

  case 694:
#line 5152 "../src/parser/XQParser.y"
    {
    (yyval.ftoption) = WRAP((yylsp[(1) - (2)]), new (MEMMGR) FTScope(FTScope::SAME, (yyvsp[(2) - (2)].ftunit), MEMMGR));
  }
    break;

  case 695:
#line 5156 "../src/parser/XQParser.y"
    {
    (yyval.ftoption) = WRAP((yylsp[(1) - (2)]), new (MEMMGR) FTScope(FTScope::DIFFERENT, (yyvsp[(2) - (2)].ftunit), MEMMGR));
  }
    break;

  case 696:
#line 5160 "../src/parser/XQParser.y"
    {
    (yyval.ftoption) = WRAP((yylsp[(1) - (2)]), new (MEMMGR) FTContent(FTContent::AT_START, MEMMGR));
  }
    break;

  case 697:
#line 5164 "../src/parser/XQParser.y"
    {
    (yyval.ftoption) = WRAP((yylsp[(1) - (2)]), new (MEMMGR) FTContent(FTContent::AT_END, MEMMGR));
  }
    break;

  case 698:
#line 5168 "../src/parser/XQParser.y"
    {
    (yyval.ftoption) = WRAP((yylsp[(1) - (2)]), new (MEMMGR) FTContent(FTContent::ENTIRE_CONTENT, MEMMGR));
  }
    break;

  case 699:
#line 5176 "../src/parser/XQParser.y"
    {
    (yyval.ftunit) = FTOption::WORDS;
  }
    break;

  case 700:
#line 5180 "../src/parser/XQParser.y"
    {
    (yyval.ftunit) = FTOption::SENTENCES;
  }
    break;

  case 701:
#line 5184 "../src/parser/XQParser.y"
    {
    (yyval.ftunit) = FTOption::PARAGRAPHS;
  }
    break;

  case 702:
#line 5192 "../src/parser/XQParser.y"
    {
    (yyval.ftunit) = FTOption::SENTENCES;
  }
    break;

  case 703:
#line 5196 "../src/parser/XQParser.y"
    {
    (yyval.ftunit) = FTOption::PARAGRAPHS;
  }
    break;

  case 714:
#line 5232 "../src/parser/XQParser.y"
    {
    CONTEXT->setFTCase(StaticContext::CASE_INSENSITIVE);
  }
    break;

  case 715:
#line 5236 "../src/parser/XQParser.y"
    {
    CONTEXT->setFTCase(StaticContext::CASE_SENSITIVE);
  }
    break;

  case 716:
#line 5240 "../src/parser/XQParser.y"
    {
    CONTEXT->setFTCase(StaticContext::LOWERCASE);
  }
    break;

  case 717:
#line 5244 "../src/parser/XQParser.y"
    {
    CONTEXT->setFTCase(StaticContext::UPPERCASE);
  }
    break;

  case 718:
#line 5253 "../src/parser/XQParser.y"
    {
    std::cerr << "diacritics insensitive" << std::endl;
  }
    break;

  case 719:
#line 5257 "../src/parser/XQParser.y"
    {
    std::cerr << "diacritics sensitive" << std::endl;
  }
    break;

  case 720:
#line 5265 "../src/parser/XQParser.y"
    {
    std::cerr << "using stemming" << std::endl;
  }
    break;

  case 721:
#line 5269 "../src/parser/XQParser.y"
    {
    std::cerr << "no stemming" << std::endl;
  }
    break;

  case 722:
#line 5279 "../src/parser/XQParser.y"
    {
    std::cerr << "using thesaurus" << std::endl;
  }
    break;

  case 723:
#line 5283 "../src/parser/XQParser.y"
    {
    std::cerr << "using thesaurus default" << std::endl;
  }
    break;

  case 724:
#line 5287 "../src/parser/XQParser.y"
    {
    std::cerr << "using thesaurus ()" << std::endl;
  }
    break;

  case 725:
#line 5291 "../src/parser/XQParser.y"
    {
    std::cerr << "using thesaurus (default)" << std::endl;
  }
    break;

  case 726:
#line 5295 "../src/parser/XQParser.y"
    {
    std::cerr << "no thesaurus" << std::endl;
  }
    break;

  case 727:
#line 5302 "../src/parser/XQParser.y"
    {
  }
    break;

  case 728:
#line 5305 "../src/parser/XQParser.y"
    {
  }
    break;

  case 729:
#line 5312 "../src/parser/XQParser.y"
    {
    std::cerr << "at StringLiteral" << std::endl;
  }
    break;

  case 730:
#line 5316 "../src/parser/XQParser.y"
    {
    std::cerr << "at StringLiteral relationship StringLiteral" << std::endl;
  }
    break;

  case 731:
#line 5320 "../src/parser/XQParser.y"
    {
    std::cerr << "at StringLiteral levels" << std::endl;
  }
    break;

  case 732:
#line 5324 "../src/parser/XQParser.y"
    {
    std::cerr << "at StringLiteral relationship StringLiteral levels" << std::endl;
  }
    break;

  case 733:
#line 5334 "../src/parser/XQParser.y"
    {
    yyerror((yylsp[(1) - (4)]), "FTStopWordOption is not supported. [err:FTST0006]");
  }
    break;

  case 734:
#line 5338 "../src/parser/XQParser.y"
    {
    yyerror((yylsp[(1) - (3)]), "FTStopWordOption is not supported. [err:FTST0006]");
  }
    break;

  case 735:
#line 5342 "../src/parser/XQParser.y"
    {
    yyerror((yylsp[(1) - (4)]), "FTStopWordOption is not supported. [err:FTST0006]");
  }
    break;

  case 736:
#line 5349 "../src/parser/XQParser.y"
    {
  }
    break;

  case 737:
#line 5352 "../src/parser/XQParser.y"
    {
  }
    break;

  case 738:
#line 5360 "../src/parser/XQParser.y"
    {
    std::cerr << "at URILiteral" << std::endl;
  }
    break;

  case 739:
#line 5364 "../src/parser/XQParser.y"
    {
    std::cerr << "()" << std::endl;
  }
    break;

  case 740:
#line 5371 "../src/parser/XQParser.y"
    {
    std::cerr << "StringLiteral" << std::endl;
  }
    break;

  case 741:
#line 5375 "../src/parser/XQParser.y"
    {
    std::cerr << ", StringLiteral" << std::endl;
  }
    break;

  case 742:
#line 5383 "../src/parser/XQParser.y"
    {
    std::cerr << "union" << std::endl;
  }
    break;

  case 743:
#line 5387 "../src/parser/XQParser.y"
    {
    std::cerr << "except" << std::endl;
  }
    break;

  case 744:
#line 5395 "../src/parser/XQParser.y"
    {
    std::cerr << "language StringLiteral" << std::endl;
  }
    break;

  case 745:
#line 5403 "../src/parser/XQParser.y"
    {
    std::cerr << "using wildcards" << std::endl;
  }
    break;

  case 746:
#line 5407 "../src/parser/XQParser.y"
    {
    std::cerr << "no wildcards" << std::endl;
  }
    break;

  case 747:
#line 5415 "../src/parser/XQParser.y"
    {
}
    break;

  case 748:
#line 5422 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = (yyvsp[(3) - (3)].astNode);
  }
    break;

  case 749:
#line 5430 "../src/parser/XQParser.y"
    {
    CHECK_SPECIFIED((yylsp[(1) - (3)]), BIT_REVALIDATION_SPECIFIED, "revalidation", "XUST0003");
    CONTEXT->setRevalidationMode(DocumentCache::VALIDATION_STRICT);
  }
    break;

  case 750:
#line 5435 "../src/parser/XQParser.y"
    {
    CHECK_SPECIFIED((yylsp[(1) - (3)]), BIT_REVALIDATION_SPECIFIED, "revalidation", "XUST0003");
    CONTEXT->setRevalidationMode(DocumentCache::VALIDATION_LAX);
  }
    break;

  case 751:
#line 5440 "../src/parser/XQParser.y"
    {
    CHECK_SPECIFIED((yylsp[(1) - (3)]), BIT_REVALIDATION_SPECIFIED, "revalidation", "XUST0003");
    CONTEXT->setRevalidationMode(DocumentCache::VALIDATION_SKIP);
  }
    break;

  case 752:
#line 5452 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (6)]), new (MEMMGR) UInsertAsFirst((yyvsp[(2) - (6)].astNode), (yyvsp[(6) - (6)].astNode), MEMMGR));
  }
    break;

  case 753:
#line 5456 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (6)]), new (MEMMGR) UInsertAsLast((yyvsp[(2) - (6)].astNode), (yyvsp[(6) - (6)].astNode), MEMMGR));
  }
    break;

  case 754:
#line 5460 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (4)]), new (MEMMGR) UInsertInto((yyvsp[(2) - (4)].astNode), (yyvsp[(4) - (4)].astNode), MEMMGR));
  }
    break;

  case 755:
#line 5464 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (4)]), new (MEMMGR) UInsertAfter((yyvsp[(2) - (4)].astNode), (yyvsp[(4) - (4)].astNode), MEMMGR));
  }
    break;

  case 756:
#line 5468 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (4)]), new (MEMMGR) UInsertBefore((yyvsp[(2) - (4)].astNode), (yyvsp[(4) - (4)].astNode), MEMMGR));
  }
    break;

  case 759:
#line 5479 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (2)]), new (MEMMGR) UDelete((yyvsp[(2) - (2)].astNode), MEMMGR));
  }
    break;

  case 762:
#line 5490 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (7)]), new (MEMMGR) UReplaceValueOf((yyvsp[(5) - (7)].astNode), (yyvsp[(7) - (7)].astNode), MEMMGR));
  }
    break;

  case 763:
#line 5494 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (5)]), new (MEMMGR) UReplace((yyvsp[(3) - (5)].astNode), (yyvsp[(5) - (5)].astNode), MEMMGR));
  }
    break;

  case 764:
#line 5504 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (5)]), new (MEMMGR) URename((yyvsp[(3) - (5)].astNode), (yyvsp[(5) - (5)].astNode), MEMMGR));
  }
    break;

  case 765:
#line 5512 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (6)]), new (MEMMGR) UTransform((yyvsp[(2) - (6)].copyBindingList), (yyvsp[(4) - (6)].astNode), (yyvsp[(6) - (6)].astNode), MEMMGR));
  }
    break;

  case 766:
#line 5519 "../src/parser/XQParser.y"
    {
    (yyvsp[(1) - (3)].copyBindingList)->push_back((yyvsp[(3) - (3)].copyBinding));
    (yyval.copyBindingList) = (yyvsp[(1) - (3)].copyBindingList);
  }
    break;

  case 767:
#line 5524 "../src/parser/XQParser.y"
    {
    (yyval.copyBindingList) = new (MEMMGR) VectorOfCopyBinding(XQillaAllocator<CopyBinding*>(MEMMGR));
    (yyval.copyBindingList)->push_back((yyvsp[(1) - (1)].copyBinding));
  }
    break;

  case 768:
#line 5532 "../src/parser/XQParser.y"
    {
    (yyval.copyBinding) = WRAP((yylsp[(1) - (4)]), new (MEMMGR) CopyBinding(MEMMGR, (yyvsp[(2) - (4)].str), (yyvsp[(4) - (4)].astNode)));
  }
    break;

  case 769:
#line 5541 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQLiteral(
                  SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                  SchemaSymbols::fgDT_INTEGER,
                  (yyvsp[(1) - (1)].str), AnyAtomicType::DECIMAL,
                  MEMMGR));
  }
    break;

  case 770:
#line 5553 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQLiteral(
                  SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                  SchemaSymbols::fgDT_DECIMAL,
                  (yyvsp[(1) - (1)].str), AnyAtomicType::DECIMAL,
                  MEMMGR));
  }
    break;

  case 771:
#line 5565 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQLiteral(
                  SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                  SchemaSymbols::fgDT_DOUBLE,
                  (yyvsp[(1) - (1)].str), AnyAtomicType::DOUBLE,
                  MEMMGR));
  }
    break;

  case 772:
#line 5578 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQLiteral(
                  SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                  SchemaSymbols::fgDT_STRING,
                  (yyvsp[(1) - (1)].str), AnyAtomicType::STRING,
                  MEMMGR));
  }
    break;

  case 773:
#line 5589 "../src/parser/XQParser.y"
    {
    // Check for a colon
    for(XMLCh *tmp = (yyvsp[(1) - (1)].str); *tmp; ++tmp)
      if(*tmp == ':') yyerror((yylsp[(1) - (1)]), "Expecting an NCName, found a QName");
  }
    break;

  case 774:
#line 5598 "../src/parser/XQParser.y"
    {
    (yyval.qName) = new (MEMMGR) QualifiedName((yyvsp[(1) - (1)].str),MEMMGR);
  }
    break;

  case 775:
#line 5610 "../src/parser/XQParser.y"
    {
    FunctionSignature *signature = new (MEMMGR) FunctionSignature(MEMMGR);
    (yyval.functDecl) = WRAP((yylsp[(1) - (5)]), new (MEMMGR) XQUserFunction((yyvsp[(4) - (5)].str), 0, signature, (yyvsp[(5) - (5)].astNode), MEMMGR));
  }
    break;

  case 776:
#line 5615 "../src/parser/XQParser.y"
    {
    FunctionSignature *signature = new (MEMMGR) FunctionSignature((yyvsp[(5) - (7)].argSpecs), (yyvsp[(6) - (7)].sequenceType), MEMMGR);
    (yyval.functDecl) = WRAP((yylsp[(1) - (7)]), new (MEMMGR) XQUserFunction((yyvsp[(4) - (7)].str), 0, signature, (yyvsp[(7) - (7)].astNode), MEMMGR));
  }
    break;

  case 777:
#line 5620 "../src/parser/XQParser.y"
    {
    FunctionSignature *signature = new (MEMMGR) FunctionSignature(MEMMGR);
    (yyval.functDecl) = WRAP((yylsp[(1) - (8)]), new (MEMMGR) XQUserFunction((yyvsp[(4) - (8)].str), (yyvsp[(7) - (8)].itemList), signature, (yyvsp[(8) - (8)].astNode), MEMMGR));
    (yyval.functDecl)->setModeList((yyvsp[(5) - (8)].modeList));
  }
    break;

  case 778:
#line 5626 "../src/parser/XQParser.y"
    {
    FunctionSignature *signature = new (MEMMGR) FunctionSignature((yyvsp[(8) - (10)].argSpecs), (yyvsp[(9) - (10)].sequenceType), MEMMGR);
    (yyval.functDecl) = WRAP((yylsp[(1) - (10)]), new (MEMMGR) XQUserFunction((yyvsp[(4) - (10)].str), (yyvsp[(7) - (10)].itemList), signature, (yyvsp[(10) - (10)].astNode), MEMMGR));
    (yyval.functDecl)->setModeList((yyvsp[(5) - (10)].modeList));
  }
    break;

  case 779:
#line 5632 "../src/parser/XQParser.y"
    {
    FunctionSignature *signature = new (MEMMGR) FunctionSignature(MEMMGR);
    (yyval.functDecl) = WRAP((yylsp[(1) - (6)]), new (MEMMGR) XQUserFunction(0, (yyvsp[(5) - (6)].itemList), signature, (yyvsp[(6) - (6)].astNode), MEMMGR));
    (yyval.functDecl)->setModeList((yyvsp[(3) - (6)].modeList));
  }
    break;

  case 780:
#line 5638 "../src/parser/XQParser.y"
    {
    FunctionSignature *signature = new (MEMMGR) FunctionSignature((yyvsp[(6) - (8)].argSpecs), (yyvsp[(7) - (8)].sequenceType), MEMMGR);
    (yyval.functDecl) = WRAP((yylsp[(1) - (8)]), new (MEMMGR) XQUserFunction(0, (yyvsp[(5) - (8)].itemList), signature, (yyvsp[(8) - (8)].astNode), MEMMGR));
    (yyval.functDecl)->setModeList((yyvsp[(3) - (8)].modeList));
  }
    break;

  case 781:
#line 5647 "../src/parser/XQParser.y"
    {
    (yyval.sequenceType) = 0;
  }
    break;

  case 782:
#line 5651 "../src/parser/XQParser.y"
    {
    (yyval.sequenceType) = (yyvsp[(2) - (2)].sequenceType);
  }
    break;

  case 783:
#line 5659 "../src/parser/XQParser.y"
    {
    (yyval.argSpecs) = 0;
  }
    break;

  case 784:
#line 5663 "../src/parser/XQParser.y"
    {
    (yyval.argSpecs) = (yyvsp[(2) - (3)].argSpecs);
  }
    break;

  case 785:
#line 5670 "../src/parser/XQParser.y"
    {
    (yyval.modeList) = new (MEMMGR) XQUserFunction::ModeList(XQillaAllocator<XQUserFunction::Mode*>(MEMMGR));
    (yyval.modeList)->push_back(WRAP((yyloc), new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::DEFAULT)));
  }
    break;

  case 786:
#line 5675 "../src/parser/XQParser.y"
    {
    (yyval.modeList) = (yyvsp[(2) - (2)].modeList);
  }
    break;

  case 787:
#line 5683 "../src/parser/XQParser.y"
    {
    (yyval.modeList) = new (MEMMGR) XQUserFunction::ModeList(XQillaAllocator<XQUserFunction::Mode*>(MEMMGR));
    (yyval.modeList)->push_back((yyvsp[(1) - (1)].mode));
  }
    break;

  case 788:
#line 5688 "../src/parser/XQParser.y"
    {
    (yyvsp[(1) - (3)].modeList)->push_back((yyvsp[(3) - (3)].mode));
    (yyval.modeList) = (yyvsp[(1) - (3)].modeList);
  }
    break;

  case 789:
#line 5697 "../src/parser/XQParser.y"
    {
    (yyval.mode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQUserFunction::Mode((yyvsp[(1) - (1)].str)));
  }
    break;

  case 790:
#line 5701 "../src/parser/XQParser.y"
    {
    (yyval.mode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::DEFAULT));
  }
    break;

  case 791:
#line 5705 "../src/parser/XQParser.y"
    {
    (yyval.mode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::ALL));
  }
    break;

  case 792:
#line 5713 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQCallTemplate((yyvsp[(3) - (3)].str), 0, MEMMGR));
  }
    break;

  case 793:
#line 5717 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (7)]), new (MEMMGR) XQCallTemplate((yyvsp[(3) - (7)].str), (yyvsp[(6) - (7)].templateArgs), MEMMGR));
  }
    break;

  case 794:
#line 5725 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQApplyTemplates((yyvsp[(3) - (3)].astNode), 0, 0, MEMMGR));
  }
    break;

  case 795:
#line 5729 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (7)]), new (MEMMGR) XQApplyTemplates((yyvsp[(3) - (7)].astNode), (yyvsp[(6) - (7)].templateArgs), 0, MEMMGR));
  }
    break;

  case 796:
#line 5733 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (5)]), new (MEMMGR) XQApplyTemplates((yyvsp[(3) - (5)].astNode), 0, (yyvsp[(5) - (5)].mode), MEMMGR));
  }
    break;

  case 797:
#line 5737 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (9)]), new (MEMMGR) XQApplyTemplates((yyvsp[(3) - (9)].astNode), (yyvsp[(8) - (9)].templateArgs), (yyvsp[(5) - (9)].mode), MEMMGR));
  }
    break;

  case 798:
#line 5745 "../src/parser/XQParser.y"
    {
    (yyval.mode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQUserFunction::Mode((yyvsp[(1) - (1)].str)));
  }
    break;

  case 799:
#line 5749 "../src/parser/XQParser.y"
    {
    (yyval.mode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::DEFAULT));
  }
    break;

  case 800:
#line 5753 "../src/parser/XQParser.y"
    {
    (yyval.mode) = WRAP((yylsp[(1) - (1)]), new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::CURRENT));
  }
    break;

  case 801:
#line 5761 "../src/parser/XQParser.y"
    {
    (yyval.templateArgs) = new (MEMMGR) TemplateArguments(XQillaAllocator<XQTemplateArgument*>(MEMMGR));
    (yyval.templateArgs)->push_back((yyvsp[(1) - (1)].templateArg));
  }
    break;

  case 802:
#line 5766 "../src/parser/XQParser.y"
    {
    (yyval.templateArgs) = (yyvsp[(1) - (3)].templateArgs);
    (yyval.templateArgs)->push_back((yyvsp[(3) - (3)].templateArg));
  }
    break;

  case 803:
#line 5776 "../src/parser/XQParser.y"
    {
    if((yyvsp[(3) - (5)].sequenceType) != 0)
      (yyvsp[(5) - (5)].astNode) = (yyvsp[(3) - (5)].sequenceType)->convertFunctionArg((yyvsp[(5) - (5)].astNode), CONTEXT, /*numericfunction*/false, (yyvsp[(3) - (5)].sequenceType));
    (yyval.templateArg) = WRAP((yylsp[(1) - (5)]), new (MEMMGR) XQTemplateArgument((yyvsp[(2) - (5)].str), (yyvsp[(5) - (5)].astNode), MEMMGR));
  }
    break;

  case 806:
#line 5793 "../src/parser/XQParser.y"
    {
    (yyval.astNode) = WRAP((yylsp[(1) - (3)]), new (MEMMGR) XQFunctionRef((yyvsp[(1) - (3)].str), atoi(UTF8((yyvsp[(3) - (3)].str))), MEMMGR));
  }
    break;

  case 807:
#line 5801 "../src/parser/XQParser.y"
    {
    FunctionSignature *signature = new (MEMMGR) FunctionSignature((yyvsp[(2) - (4)].argSpecs), (yyvsp[(3) - (4)].sequenceType), MEMMGR);
    XQUserFunction *func = WRAP((yylsp[(1) - (4)]), new (MEMMGR) XQUserFunction(0, signature, (yyvsp[(4) - (4)].astNode), false, MEMMGR));
    (yyval.astNode) = WRAP((yylsp[(1) - (4)]), new (MEMMGR) XQInlineFunction(func, MEMMGR));
  }
    break;

  case 808:
#line 5812 "../src/parser/XQParser.y"
    {
    REJECT_NOT_VERSION3(DynamicFunctionInvocation, (yylsp[(1) - (3)]));
    (yyval.astNode) = WRAP((yylsp[(2) - (3)]), new (MEMMGR) XQFunctionDeref((yyvsp[(1) - (3)].astNode), new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR)), MEMMGR));
  }
    break;

  case 809:
#line 5817 "../src/parser/XQParser.y"
    {
    REJECT_NOT_VERSION3(DynamicFunctionInvocation, (yylsp[(1) - (4)]));

    bool partial = false;
    VectorOfASTNodes::iterator i;
    for(i = (yyvsp[(3) - (4)].itemList)->begin(); i != (yyvsp[(3) - (4)].itemList)->end(); ++i) {
      if(*i == 0) {
        partial = true;
        break;
      }
    }

    if(partial) {
      // This is a partial function application
      (yyval.astNode) = WRAP((yylsp[(2) - (4)]), new (MEMMGR) XQPartialApply((yyvsp[(1) - (4)].astNode), (yyvsp[(3) - (4)].itemList), MEMMGR));
    }
    else {
      (yyval.astNode) = WRAP((yylsp[(2) - (4)]), new (MEMMGR) XQFunctionDeref((yyvsp[(1) - (4)].astNode), (yyvsp[(3) - (4)].itemList), MEMMGR));
    }
  }
    break;

  case 812:
#line 5845 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_FUNCTION);
  }
    break;

  case 813:
#line 5853 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType(new (MEMMGR) VectorOfSequenceTypes(XQillaAllocator<SequenceType*>(MEMMGR)), (yyvsp[(5) - (5)].sequenceType));
  }
    break;

  case 814:
#line 5857 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = new (MEMMGR) SequenceType::ItemType((yyvsp[(3) - (6)].sequenceTypes), (yyvsp[(6) - (6)].sequenceType));
  }
    break;

  case 815:
#line 5864 "../src/parser/XQParser.y"
    {
    (yyval.sequenceTypes) = new (MEMMGR) VectorOfSequenceTypes(XQillaAllocator<SequenceType*>(MEMMGR));
    (yyval.sequenceTypes)->push_back((yyvsp[(1) - (1)].sequenceType));
  }
    break;

  case 816:
#line 5869 "../src/parser/XQParser.y"
    {
    (yyval.sequenceTypes) = (yyvsp[(1) - (3)].sequenceTypes);
    (yyval.sequenceTypes)->push_back((yyvsp[(3) - (3)].sequenceType));
  }
    break;

  case 817:
#line 5878 "../src/parser/XQParser.y"
    {
    (yyval.itemType) = (yyvsp[(2) - (3)].itemType);
  }
    break;


/* Line 1267 of yacc.c.  */
#line 10680 "../src/parser/XQParser.cpp"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[0] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the look-ahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 5913 "../src/parser/XQParser.y"


}  // namespace XQParser


