AUTOMAKE_OPTIONS = foreign dist-zip
SUBDIRS = include

INCLUDES = -I$(top_srcdir)/include/ $(xerces_include) -I$(top_srcdir)/src/lexer/ $(faxpp_include) $(tidy_include)
LDFLAGS = -L$(xerces_lib) -R$(xerces_lib) $(faxpp_lib) $(tidy_lib) -lxerces-c $(faxpp_library) $(tidy_library)
LDADD = libxqilla.la
ACLOCAL_AMFLAGS = -I autotools/m4

bin_PROGRAMS = xqilla
noinst_PROGRAMS = xqtsRunner xmarkRunner simple-basic simple-context-item xqc-basic xqc-context-item compile-delayed-module

if BUILD_DOM_SAMPLES
noinst_PROGRAMS += dom-basic dom-context-item dom-resolver
endif

lib_LTLIBRARIES = libxqilla.la

# Library version, see http://sourceware.org/autobook/autobook/autobook_91.html
# -no-undefined for Cygwin (bug #1639669)
libxqilla_la_LDFLAGS = -version-info 6:0:3 -no-undefined

libxqilla_la_SOURCES = \
src/framework/ReferenceCounted.cpp \
src/framework/BaseMemoryManager.cpp \
src/framework/ProxyMemoryManager.cpp \
src/framework/StringPool.cpp \
src/framework/XPath2MemoryManagerImpl.cpp \
src/context/UpdateFactory.cpp \
src/context/impl/CollationImpl.cpp \
src/context/impl/CodepointCollation.cpp \
src/context/impl/VarStoreImpl.cpp \
src/context/impl/VarTypeStoreImpl.hpp \
src/context/impl/VarTypeStoreImpl.cpp \
src/context/impl/VarHashEntryImpl.cpp \
src/context/impl/XQContextImpl.cpp \
src/context/impl/XQDynamicContextImpl.cpp \
src/context/impl/ItemFactoryImpl.cpp \
src/ast/XQAtomize.cpp \
src/ast/XQEffectiveBooleanValue.cpp \
src/ast/XQMap.cpp \
src/ast/XQAnalyzeString.cpp \
src/ast/XQCastableAs.cpp \
src/ast/XQCastAs.cpp \
src/ast/XQContextItem.cpp \
src/ast/XQCopy.cpp \
src/ast/XQCopyOf.cpp \
src/ast/XQFunction.cpp \
src/ast/XQIf.cpp \
src/ast/ASTNodeImpl.cpp \
src/ast/XQInlineFunction.cpp \
src/ast/XQLiteral.cpp \
src/ast/XQNav.cpp \
src/ast/XQOperator.cpp \
src/ast/XQSequence.cpp \
src/ast/XQStep.cpp \
src/ast/XQTreatAs.cpp \
src/ast/XQFunctionCoercion.cpp \
src/ast/XQVariable.cpp \
src/ast/StaticAnalysis.cpp \
src/ast/StaticType.cpp \
src/ast/NumericFunction.cpp \
src/ast/XQDOMConstructor.cpp \
src/ast/XQDocumentConstructor.cpp \
src/ast/XQElementConstructor.cpp \
src/ast/XQAttributeConstructor.cpp \
src/ast/XQNamespaceConstructor.cpp \
src/ast/XQPIConstructor.cpp \
src/ast/XQCommentConstructor.cpp \
src/ast/XQTextConstructor.cpp \
src/ast/XQFunctionCall.cpp \
src/ast/XQFunctionConversion.cpp \
src/ast/XQFunctionDeref.cpp \
src/ast/XQPartialApply.cpp \
src/ast/XQFunctionRef.cpp \
src/ast/XQGlobalVariable.cpp \
src/ast/XQOrderingChange.cpp \
src/ast/XQQuantified.cpp \
src/ast/XQValidate.cpp \
src/ast/XQTypeswitch.cpp \
src/ast/ConvertFunctionArg.cpp \
src/ast/XQDocumentOrder.cpp \
src/ast/XQPredicate.cpp \
src/ast/XQReturn.cpp \
src/ast/TupleNode.cpp \
src/ast/ContextTuple.cpp \
src/ast/ForTuple.cpp \
src/ast/LetTuple.cpp \
src/ast/WhereTuple.cpp \
src/ast/CountTuple.cpp \
src/ast/OrderByTuple.cpp \
src/ast/XQNamespaceBinding.cpp \
src/ast/XQCallTemplate.cpp \
src/ast/XQApplyTemplates.cpp \
src/debug/ASTDebugHook.cpp \
src/debug/DebugHookDecorator.cpp \
src/debug/TupleDebugHook.cpp \
src/debug/InputParser.cpp \
src/debug/InteractiveDebugger.cpp \
src/debug/StackFrame.cpp \
src/events/ContentSequenceFilter.cpp \
src/events/EventGenerator.cpp \
src/events/EventSerializer.cpp \
src/events/NoInheritFilter.cpp \
src/events/NoInheritFilter.hpp \
src/events/NSFixupFilter.cpp \
src/events/QueryPathTreeFilter.cpp \
src/fulltext/FTContains.cpp \
src/fulltext/FTWords.cpp \
src/fulltext/FTOr.cpp \
src/fulltext/FTAnd.cpp \
src/fulltext/FTMildnot.cpp \
src/fulltext/FTUnaryNot.cpp \
src/fulltext/DefaultTokenizer.cpp \
src/fulltext/DefaultTokenStore.cpp \
src/fulltext/FTOrder.cpp \
src/fulltext/FTDistance.cpp \
src/fulltext/FTScope.cpp \
src/fulltext/FTContent.cpp \
src/fulltext/FTWindow.cpp \
src/dom-api/impl/XQillaBuilderImpl.cpp \
src/dom-api/impl/XQillaBuilderImpl.hpp \
src/dom-api/impl/XQillaDocumentImpl.cpp \
src/dom-api/impl/XQillaDocumentImpl.hpp \
src/dom-api/impl/XQillaExpressionImpl.cpp \
src/dom-api/impl/XQillaExpressionImpl.hpp \
src/dom-api/impl/XQillaNSResolverImpl.cpp \
src/dom-api/impl/XQillaXMLGrammarPoolImpl.cpp \
src/dom-api/impl/XQillaXMLGrammarPoolImpl.hpp \
src/dom-api/impl/XPathDocumentImpl.cpp \
src/dom-api/impl/XPathDocumentImpl.hpp \
src/dom-api/impl/XPathNamespaceImpl.cpp \
src/dom-api/impl/XPathNamespaceImpl.hpp \
src/dom-api/impl/XPath2ResultImpl.cpp \
src/dom-api/impl/XPath2ResultImpl.hpp \
src/dom-api/XQillaImplementation.cpp \
src/dom-api/XQillaImplementation.hpp \
src/dom-api/XPath2NodeSerializer.cpp \
src/dom-api/XPath2NodeSerializer.hpp \
src/simple-api/XQilla.cpp \
src/simple-api/XQQuery.cpp \
src/utils/UTF8Str.cpp \
src/utils/XMLChCompare.cpp \
src/utils/XStr.cpp \
src/utils/ContextUtils.cpp \
src/utils/DateUtils.cpp \
src/utils/DateUtils.hpp \
src/utils/NumUtils.cpp \
src/utils/XQillaPlatformUtils.cpp \
src/utils/XPath2NSUtils.cpp \
src/utils/XPath2Utils.cpp \
src/utils/UnicodeTransformer.cpp \
src/utils/utf8proc/utf8proc.c \
src/utils/utf8proc/utf8proc_data.h \
src/utils/DelayedModule.cpp \
src/exceptions/XQillaException.cpp \
src/exceptions/XQException.cpp \
src/exceptions/InvalidLexicalSpaceException.hpp \
src/exceptions/MiscException.hpp \
src/functions/EXSLTMathFunction.cpp \
src/functions/ExternalFunction.cpp \
src/functions/XQillaFunction.cpp \
src/functions/FunctionParseXML.cpp \
src/functions/FuncFactory.cpp \
src/functions/FunctionLookup.cpp \
src/functions/FunctionAbs.cpp \
src/functions/FunctionAcos.cpp \
src/functions/FunctionAsin.cpp \
src/functions/FunctionAtan.cpp \
src/functions/FunctionBaseURI.cpp \
src/functions/FunctionCaseFold.cpp \
src/functions/FunctionCeiling.cpp \
src/functions/FunctionCodepointsToString.cpp \
src/functions/FunctionCollection.cpp \
src/functions/FunctionCompare.cpp \
src/functions/FunctionConcat.cpp \
src/functions/FunctionContains.cpp \
src/functions/FunctionCount.cpp \
src/functions/FunctionCos.cpp \
src/functions/FunctionCurrentDate.cpp \
src/functions/FunctionCurrentDateTime.cpp \
src/functions/FunctionCurrentTime.cpp \
src/functions/FunctionDateTime.cpp \
src/functions/FunctionDefaultCollation.cpp \
src/functions/FunctionDistinctValues.cpp \
src/functions/FunctionDocAvailable.cpp \
src/functions/FunctionDoc.cpp \
src/functions/FunctionDocument.cpp \
src/functions/FunctionDocumentURI.cpp \
src/functions/FunctionEmpty.cpp \
src/functions/FunctionEndsWith.cpp \
src/functions/FunctionError.cpp \
src/functions/FunctionExp.cpp \
src/functions/FunctionFloor.cpp \
src/functions/FunctionFunctionArity.cpp \
src/functions/FunctionFunctionName.cpp \
src/functions/FunctionId.cpp \
src/functions/FunctionIdref.cpp \
src/functions/FunctionImplicitTimezone.cpp \
src/functions/FunctionLang.cpp \
src/functions/FunctionLast.cpp \
src/functions/FunctionLocalname.cpp \
src/functions/FunctionLocalNameFromQName.cpp \
src/functions/FunctionLog.cpp \
src/functions/FunctionLowerCase.cpp \
src/functions/FunctionMatches.cpp \
src/functions/FunctionName.cpp \
src/functions/FunctionNamespaceUri.cpp \
src/functions/FunctionNamespaceURIFromQName.cpp \
src/functions/FunctionNilled.cpp \
src/functions/FunctionNodeName.cpp \
src/functions/FunctionNormalizeSpace.cpp \
src/functions/FunctionNormalizeUnicode.cpp \
src/functions/FunctionNot.cpp \
src/functions/FunctionNumber.cpp \
src/functions/FunctionPosition.cpp \
src/functions/FunctionPower.cpp \
src/functions/FunctionPrefixFromQName.cpp \
src/functions/FunctionQName.cpp \
src/functions/FunctionRegexGroup.cpp \
src/functions/FunctionReplace.cpp \
src/functions/FunctionResolveURI.cpp \
src/functions/FunctionRoot.cpp \
src/functions/FunctionRound.cpp \
src/functions/FunctionRoundHalfToEven.cpp \
src/functions/FunctionSin.cpp \
src/functions/FunctionSqrt.cpp \
src/functions/FunctionTan.cpp \
src/functions/FunctionSentences.cpp \
src/functions/FunctionStartsWith.cpp \
src/functions/FunctionStaticBaseURI.cpp \
src/functions/FunctionString.cpp \
src/functions/FunctionStringLength.cpp \
src/functions/FunctionStringToCodepoints.cpp \
src/functions/FunctionSubstring.cpp \
src/functions/FunctionTime.cpp \
src/functions/FunctionTokenize.cpp \
src/functions/FunctionTrace.cpp \
src/functions/FunctionHead.cpp \
src/functions/FunctionTail.cpp \
src/functions/FunctionUnordered.cpp \
src/functions/FunctionUnparsedText.cpp \
src/functions/FunctionUpperCase.cpp \
src/functions/FunctionWords.cpp \
src/functions/FunctionAdjustDateTimeToTimezone.cpp \
src/functions/FunctionAdjustDateToTimezone.cpp \
src/functions/FunctionAdjustTimeToTimezone.cpp \
src/functions/FunctionDayFromDate.cpp \
src/functions/FunctionDayFromDateTime.cpp \
src/functions/FunctionDaysFromDuration.cpp \
src/functions/FunctionHoursFromDateTime.cpp \
src/functions/FunctionHoursFromDuration.cpp \
src/functions/FunctionHoursFromTime.cpp \
src/functions/FunctionMinutesFromDateTime.cpp \
src/functions/FunctionMinutesFromDuration.cpp \
src/functions/FunctionMinutesFromTime.cpp \
src/functions/FunctionMonthFromDate.cpp \
src/functions/FunctionMonthFromDateTime.cpp \
src/functions/FunctionMonthsFromDuration.cpp \
src/functions/FunctionSecondsFromDateTime.cpp \
src/functions/FunctionSecondsFromDuration.cpp \
src/functions/FunctionSecondsFromTime.cpp \
src/functions/FunctionTimezoneFromDate.cpp \
src/functions/FunctionTimezoneFromDateTime.cpp \
src/functions/FunctionTimezoneFromTime.cpp \
src/functions/FunctionYearFromDate.cpp \
src/functions/FunctionYearFromDateTime.cpp \
src/functions/FunctionYearsFromDuration.cpp \
src/functions/FunctionParseJSON.cpp \
src/functions/FunctionSerializeJSON.cpp \
src/functions/FunctionParseHTML.cpp \
src/functions/FunctionExplain.cpp \
src/functions/FunctionAnalyzeString.cpp \
src/functions/RegExpFunction.cpp \
src/functions/XQUserFunction.cpp \
src/functions/FunctionSignature.cpp \
src/functions/BuiltInModules.cpp \
src/functions/CoreModule.hpp \
src/functions/FnModule.hpp \
src/items/impl/ATAnySimpleTypeImpl.cpp \
src/items/impl/ATAnySimpleTypeImpl.hpp \
src/items/impl/ATAnyURIOrDerivedImpl.cpp \
src/items/impl/ATAnyURIOrDerivedImpl.hpp \
src/items/impl/ATBase64BinaryOrDerivedImpl.cpp \
src/items/impl/ATBase64BinaryOrDerivedImpl.hpp \
src/items/impl/ATBooleanOrDerivedImpl.cpp \
src/items/impl/ATDateOrDerivedImpl.cpp \
src/items/impl/ATDateTimeOrDerivedImpl.cpp \
src/items/impl/ATDecimalOrDerivedImpl.cpp \
src/items/impl/ATDoubleOrDerivedImpl.cpp \
src/items/impl/ATDurationOrDerivedImpl.cpp \
src/items/impl/ATFloatOrDerivedImpl.cpp \
src/items/impl/ATGDayOrDerivedImpl.cpp \
src/items/impl/ATGMonthDayOrDerivedImpl.cpp \
src/items/impl/ATGMonthOrDerivedImpl.cpp \
src/items/impl/ATGYearMonthOrDerivedImpl.cpp \
src/items/impl/ATGYearOrDerivedImpl.cpp \
src/items/impl/ATHexBinaryOrDerivedImpl.cpp \
src/items/impl/ATHexBinaryOrDerivedImpl.hpp \
src/items/impl/ATNotationOrDerivedImpl.cpp \
src/items/impl/ATNotationOrDerivedImpl.hpp \
src/items/impl/ATQNameOrDerivedImpl.cpp \
src/items/impl/ATStringOrDerivedImpl.cpp \
src/items/impl/ATStringOrDerivedImpl.hpp \
src/items/impl/ATTimeOrDerivedImpl.cpp \
src/items/impl/ATUntypedAtomicImpl.cpp \
src/items/impl/ATUntypedAtomicImpl.hpp \
src/items/impl/FunctionRefImpl.cpp \
src/items/impl/FunctionRefImpl.hpp \
src/items/DatatypeFactory.cpp \
src/items/DatatypeFactoryTemplate.hpp \
src/items/DatatypeLookup.cpp \
src/items/AnyAtomicType.cpp \
src/items/ATDurationOrDerived.cpp \
src/items/ATUntypedAtomic.cpp \
src/items/DateOrTimeType.cpp \
src/items/Item.cpp \
src/items/Node.cpp \
src/items/Numeric.cpp \
src/items/Timezone.cpp \
src/parser/QName.cpp \
src/parser/XQParser.cpp \
src/parser/XQParser.hpp \
src/lexer/FlexLexer.h \
src/lexer/XQLexer.cpp \
src/lexer/XQLexer2.cpp \
src/lexer/XQLexer.hpp \
src/lexer/XSLT2Lexer.cpp \
src/lexer/XSLT2Lexer.hpp \
src/mapm/mapm5sin.c \
src/mapm/mapm_add.c \
src/mapm/mapmasin.c \
src/mapm/mapmasn0.c \
src/mapm/mapmcbrt.c \
src/mapm/mapmcnst.c \
src/mapm/mapm_cpi.c \
src/mapm/m_apm.cpp \
src/mapm/mapm_div.c \
src/mapm/mapm_exp.c \
src/mapm/mapmfact.c \
src/mapm/mapm_fam.c \
src/mapm/mapm_fft.c \
src/mapm/mapm_flr.c \
src/mapm/mapmfmul.c \
src/mapm/mapm_fpf.c \
src/mapm/mapm_gcd.c \
src/mapm/mapmgues.c \
src/mapm/mapmhasn.c \
src/mapm/mapmhsin.c \
src/mapm/mapmipwr.c \
src/mapm/mapmistr.c \
src/mapm/m_apm_lc.h \
src/mapm/mapm_lg2.c \
src/mapm/mapm_lg3.c \
src/mapm/mapm_log.c \
src/mapm/mapm_mt.cpp \
src/mapm/mapm_mul.c \
src/mapm/mapm_pow.c \
src/mapm/mapmpwr2.c \
src/mapm/mapm_rcp.c \
src/mapm/mapm_rnd.c \
src/mapm/mapmrsin.c \
src/mapm/mapm_set.c \
src/mapm/mapm_sin.c \
src/mapm/mapmsqrt.c \
src/mapm/mapmstck.c \
src/mapm/mapmutil.c \
src/mapm/mapmutl2.c \
src/mapm/README \
src/axis/NodeTest.cpp \
src/operators/And.cpp \
src/operators/ArithmeticOperator.cpp \
src/operators/ComparisonOperator.cpp \
src/operators/Divide.cpp \
src/operators/Equals.cpp \
src/operators/Except.cpp \
src/operators/GeneralComp.cpp \
src/operators/GreaterThan.cpp \
src/operators/GreaterThanEqual.cpp \
src/operators/IntegerDivide.cpp \
src/operators/Intersect.cpp \
src/operators/LessThan.cpp \
src/operators/LessThanEqual.cpp \
src/operators/Minus.cpp \
src/operators/Mod.cpp \
src/operators/Multiply.cpp \
src/operators/NodeComparison.cpp \
src/operators/NotEquals.cpp \
src/operators/Or.cpp \
src/operators/OrderComparison.cpp \
src/operators/Plus.cpp \
src/operators/UnaryMinus.cpp \
src/operators/Union.cpp \
src/schema/DocumentCacheImpl.cpp \
src/schema/SequenceType.cpp \
src/schema/AnyAtomicTypeDatatypeValidator.cpp \
src/schema/SchemaValidatorFilter.cpp \
src/schema/FaxppDocumentCacheImpl.hpp \
src/schema/FaxppDocumentCacheImpl.cpp \
src/runtime/ClosureResult.cpp \
src/runtime/EmptyResult.cpp \
src/runtime/ResultBuffer.cpp \
src/runtime/ResultBufferImpl.cpp \
src/runtime/Result.cpp \
src/runtime/ResultImpl.cpp \
src/runtime/SequenceResult.cpp \
src/runtime/Sequence.cpp \
src/update/FunctionPut.cpp \
src/update/PendingUpdateList.cpp \
src/update/UApplyUpdates.cpp \
src/update/UDelete.cpp \
src/update/URename.cpp \
src/update/UReplace.cpp \
src/update/UReplaceValueOf.cpp \
src/update/UInsertAsFirst.cpp \
src/update/UInsertAsLast.cpp \
src/update/UInsertInto.cpp \
src/update/UInsertAfter.cpp \
src/update/UInsertBefore.cpp \
src/update/UTransform.cpp \
src/config/xqilla_config.h \
src/xqts/TestSuiteParser.cpp \
src/xqts/TestSuiteResultListener.cpp \
src/xqts/TestSuiteRunner.cpp \
src/fastxdm/FastXDMUpdateFactory.hpp \
src/fastxdm/FastXDMUpdateFactory.cpp \
src/fastxdm/FastXDMDocument.hpp \
src/fastxdm/FastXDMDocument.cpp \
src/fastxdm/FastXDMNodeImpl.hpp \
src/fastxdm/FastXDMNodeImpl.cpp \
src/fastxdm/FastXDMConfiguration.cpp \
src/fastxdm/FastXDMSequenceBuilder.hpp \
src/fastxdm/FastXDMSequenceBuilder.cpp \
src/xerces/AncestorAxis.hpp \
src/xerces/AncestorOrSelfAxis.hpp \
src/xerces/AttributeAxis.hpp \
src/xerces/Axis.hpp \
src/xerces/ChildAxis.hpp \
src/xerces/DescendantAxis.hpp \
src/xerces/DescendantOrSelfAxis.hpp \
src/xerces/FollowingAxis.hpp \
src/xerces/FollowingSiblingAxis.hpp \
src/xerces/NamespaceAxis.hpp \
src/xerces/ParentAxis.hpp \
src/xerces/PrecedingAxis.hpp \
src/xerces/PrecedingSiblingAxis.hpp \
src/xerces/AncestorAxis.cpp \
src/xerces/AncestorOrSelfAxis.cpp \
src/xerces/AttributeAxis.cpp \
src/xerces/Axis.cpp \
src/xerces/ChildAxis.cpp \
src/xerces/DescendantAxis.cpp \
src/xerces/DescendantOrSelfAxis.cpp \
src/xerces/FollowingAxis.cpp \
src/xerces/FollowingSiblingAxis.cpp \
src/xerces/NamespaceAxis.cpp \
src/xerces/ParentAxis.cpp \
src/xerces/PrecedingAxis.cpp \
src/xerces/PrecedingSiblingAxis.cpp \
src/xerces/XercesConfiguration.cpp \
src/xerces/XercesNodeImpl.hpp \
src/xerces/XercesNodeImpl.cpp \
src/xerces/XercesSequenceBuilder.hpp \
src/xerces/XercesSequenceBuilder.cpp \
src/xerces/XercesUpdateFactory.hpp \
src/xerces/XercesUpdateFactory.cpp \
src/xerces/XercesURIResolver.hpp \
src/xerces/XercesURIResolver.cpp \
src/optimizer/Optimizer.cpp \
src/optimizer/ASTCopier.cpp \
src/optimizer/ASTReleaser.cpp \
src/optimizer/ASTVisitor.cpp \
src/optimizer/PartialEvaluator.cpp \
src/optimizer/StaticTyper.cpp \
src/optimizer/QueryPathNode.cpp \
src/optimizer/QueryPathTreeGenerator.cpp \
src/optimizer/ASTToXML.cpp \
src/yajl/yajl_buf.c \
src/yajl/yajl_buf.h \
src/yajl/yajl.c \
src/yajl/yajl_encode.c \
src/yajl/yajl_encode.h \
src/yajl/yajl_gen.c \
src/yajl/yajl_lex.c \
src/yajl/yajl_lex.h \
src/yajl/yajl_parser.c \
src/yajl/yajl_parser.h \
src/yajl/yajl_common.h \
src/yajl/yajl_gen.h \
src/yajl/yajl_parse.h \
src/xqc/XQillaXQCImplementation.cpp \
src/xqc/XQillaXQCImplementation.hpp \
src/xqc/XQillaXQCStaticContext.cpp \
src/xqc/XQillaXQCStaticContext.hpp \
src/xqc/XQillaXQCDynamicContext.cpp \
src/xqc/XQillaXQCDynamicContext.hpp \
src/xqc/XQillaXQCExpression.cpp \
src/xqc/XQillaXQCExpression.hpp \
src/xqc/XQillaXQCSequence.cpp \
src/xqc/XQillaXQCSequence.hpp \
src/xqc/XQillaXQCInputStream.hpp

EXTRA_DIST = \
src/lexer/flex.skl \
src/lexer/XQLexer.l \
src/parser/XQParser.y \
src/parser/XQParser.y \
src/samples/dom-api/data.xml \
src/samples/dom-api/data.xsd \
src/samples/dom-api/foo.xml \
src/samples/simple-api/foo.xml \
src/functions/CoreModule.xq \
src/functions/FnModule.xq \
LICENSE \
TODO \
README \
ChangeLog \
docs/Doxyfile.simpleapi.in \
docs/Doxyfile.dom3api.in \
docs/Doxyfile.devapi.in \
docs/Doxyfile.xqc.in \
docs/dom3_header.html \
docs/simple_header.html \
docs/xqc_header.html \
docs/simple-api \
docs/dom3-api \
docs/xqc-api \
Win32Projects/VC11/dom-basic.vcxproj \
Win32Projects/VC11/dom-context-item.vcxproj \
Win32Projects/VC11/dom-resolver.vcxproj \
Win32Projects/VC11/simple-basic.vcxproj \
Win32Projects/VC11/simple-context-item.vcxproj \
Win32Projects/VC11/xmarkRunner.vcxproj \
Win32Projects/VC11/xqc-basic.vcxproj \
Win32Projects/VC11/xqc-context-item.vcxproj \
Win32Projects/VC11/xqilla.vcxproj \
Win32Projects/VC11/xqilla_cmd.vcxproj \
Win32Projects/VC11/xqtsRunner.vcxproj \
Win32Projects/VC11/XQilla.sln \
Win32Projects/VC10/dom-basic.vcxproj \
Win32Projects/VC10/dom-context-item.vcxproj \
Win32Projects/VC10/dom-resolver.vcxproj \
Win32Projects/VC10/simple-basic.vcxproj \
Win32Projects/VC10/simple-context-item.vcxproj \
Win32Projects/VC10/xmarkRunner.vcxproj \
Win32Projects/VC10/xqc-basic.vcxproj \
Win32Projects/VC10/xqc-context-item.vcxproj \
Win32Projects/VC10/xqilla.vcxproj \
Win32Projects/VC10/xqilla_cmd.vcxproj \
Win32Projects/VC10/xqtsRunner.vcxproj \
Win32Projects/VC10/XQilla.sln \
Win32Projects/VC9/dom-basic.vcproj \
Win32Projects/VC9/dom-context-item.vcproj \
Win32Projects/VC9/dom-resolver.vcproj \
Win32Projects/VC9/simple-basic.vcproj \
Win32Projects/VC9/simple-context-item.vcproj \
Win32Projects/VC9/xmarkRunner.vcproj \
Win32Projects/VC9/xqc-basic.vcproj \
Win32Projects/VC9/xqc-context-item.vcproj \
Win32Projects/VC9/xqilla.vcproj \
Win32Projects/VC9/xqilla_cmd.vcproj \
Win32Projects/VC9/xqtsRunner.vcproj \
Win32Projects/VC9/XQilla.sln \
tests/xqts/xqts_testsuite/errors.xml \
tests/xqts/xqts_testsuite/update_errors.xml \
tests/xqts/xqts_testsuite/xsl_errors.xml \
wintools/lib_paths.sed \
wintools/s_win32 \
wintools/README \
wintools/s_projects \
wintools/genproject.xq \
wintools/xqilla.template.xml \
wintools/xqilla.vs2010.xq \
wintools/xqilla.vs2010_filter.xq


xqilla_SOURCES = \
src/samples/xqilla/xqilla-commandline.cpp

xqtsRunner_SOURCES = \
tests/xqts/runner.cpp

xmarkRunner_SOURCES = \
tests/xmark/xmark.cpp

simple_basic_SOURCES = \
src/samples/simple-api/simple-basic.cpp

simple_context_item_SOURCES = \
src/samples/simple-api/simple-context-item.cpp

dom_basic_SOURCES = \
src/samples/dom-api/dom-basic.cpp

dom_context_item_SOURCES = \
src/samples/dom-api/dom-context-item.cpp

dom_resolver_SOURCES = \
src/samples/dom-api/dom-resolver.cpp

# Dummy C++ source to cause C++ linking.
nodist_EXTRA_xqc_basic_SOURCES = dummy.cpp
xqc_basic_SOURCES = \
src/samples/xqc-api/xqc-basic.c

# Dummy C++ source to cause C++ linking.
nodist_EXTRA_xqc_context_item_SOURCES = dummy.cpp
xqc_context_item_SOURCES = \
src/samples/xqc-api/xqc-context-item.c

compile_delayed_module_SOURCES = \
src/tools/compile-delayed-module.cpp

$(top_srcdir)/src/functions/CoreModule.hpp: $(top_srcdir)/src/functions/CoreModule.xq
	./compile-delayed-module $(top_srcdir)/src/functions/CoreModule.xq >$(top_srcdir)/src/functions/CoreModule.hpp

$(top_srcdir)/src/functions/FnModule.hpp: $(top_srcdir)/src/functions/FnModule.xq
	./compile-delayed-module $(top_srcdir)/src/functions/FnModule.xq >$(top_srcdir)/src/functions/FnModule.hpp

LEXERFILE   = $(top_srcdir)/src/lexer/XQLexer.cpp
LEXTEMPLATE = $(top_srcdir)/src/lexer/XQLexer.l
LEXSKEL     = $(top_srcdir)/src/lexer/flex.skl

PARSERFILE  = $(top_srcdir)/src/parser/XQParser.y
PARSERCPP   = $(top_srcdir)/src/parser/XQParser.cpp
PARSERHPP   = $(top_srcdir)/src/parser/XQParser.hpp

$(LEXERFILE): $(LEXTEMPLATE) $(LEXSKEL)
	$(LEX) -BUs -Pxq -S$(LEXSKEL) -o$(LEXERFILE) $(LEXTEMPLATE) # This can take a long time

$(PARSERCPP) $(PARSERHPP): $(PARSERFILE)
	$(YACC) -d -v -o $(PARSERCPP) $(PARSERFILE)

.PHONY: test docs

docs:
	$(MAKE) -C include docs

docsclean:
	$(MAKE) -C include docsclean

devdocs:
	$(MAKE) -C include devdocs

devdocsclean:
	$(MAKE) -C include devdocsclean

# Target to help keep the windows projects up-to-date
winprojects: winprojects.xq xqilla
	$(MAKE) -C include winprojects
	./xqilla winprojects.xq > winprojects_files.xml

winprojects.xq: $(libxqilla_la_SOURCES)
	@echo "let \$$filedump := <filedump>" >winprojects.xq
	@list='$(libxqilla_la_SOURCES)'; for f in $$list; do \
    echo "  <file name='$$f'/>" >>winprojects.xq; \
  done
	@echo "</filedump>" >>winprojects.xq
	@echo "for \$$dir in distinct-values(\$$filedump//file/@name/substring-before(substring-after(., '/'), '/'))" >>winprojects.xq
	@echo "order by \$$dir" >>winprojects.xq
	@echo "return document{'      ', <filter name='{\$$dir}'>{" >>winprojects.xq
	@echo "  for \$$f in \$$filedump/file[contains(@name, \$$dir)]" >>winprojects.xq
	@echo "  order by \$$f/@name" >>winprojects.xq
	@echo "  return ('&#10;        ', \$$f)" >>winprojects.xq
	@echo "}&#10;      </filter>}" >>winprojects.xq
