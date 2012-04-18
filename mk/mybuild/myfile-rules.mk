# Generated by GOLD Parser Builder using Mybuild program template.

# Rule productions for 'MyFile' grammar.

#
# As for symbols each rule can have a constructor that is used to produce an
# application-specific representation of the rule data.
# Production functions are named '$(gold_grammar)_produce-<ID>' and have the
# following signature:
#
# Params:
#   1..N: Each argument contains a value of the corresponding symbol in the RHS
#         of the rule production.
#
# Return:
#   The value to pass as an argument to a rule containing the production
#   of this rule in its RHS, or to return to user in case of the Start Symbol.
#
# If production function is not defined then the rule is produced by
# concatenating the RHS through spaces. To reuse this default value one can
# call 'gold_default_produce' function.
#

# Rule: <MyFile> ::= <Package> <Imports> <Entities>
# Args: 1..3 - Symbols in the RHS.
define $(gold_grammar)_produce-MyFile
	$(for root <- $(new MyFileContentRoot),
		$(set root->name,$1)
		$(set root->imports,$2)
		$(set root->types,$3)
		$(root))
endef

# Rule: <Package> ::= package <QualifiedName>
# Args: 1..2 - Symbols in the RHS.
$(gold_grammar)_produce-Package_package  = $2

# Rule: <Package> ::=
# Args: 1..0 - Symbols in the RHS.
define $(gold_grammar)_produce-Package
	$(call gold_report_warning,
			Using default package)
endef

# Rule: <Import> ::= import <QualifiedNameWithWildcard>
# Args: 1..2 - Symbols in the RHS.
$(gold_grammar)_produce-Import_import = $2

# Rule: <AnnotatedType> ::= <Annotations> <Type>
# Args: 1..2 - Symbols in the RHS.
define $(gold_grammar)_produce-AnnotatedType
	$(for target <- $2,
		$(set+ target->annotations,$1)
		$(target))
endef

# Rule: <AnnotationType> ::= annotation Identifier '{' <AnnotationMembers> '}'
# Args: 1..5 - Symbols in the RHS.
define $(gold_grammar)_produce-AnnotationType_annotation_Identifier_LBrace_RBrace
	$(foreach type,$(new MyAnnotationType),
		$(set type->name,$2)
		$(set type->origin,$(call gold_location_of,2))

		$(set type->options,$4)

		$(type)
	)
endef

# Rule: <AnnotatedAnnotationMember> ::= <Annotations> <Option>
# Args: 1..2 - Symbols in the RHS.
define $(gold_grammar)_produce-AnnotatedAnnotationMember
	$(for target <- $2,
		$(set+ target->annotations,$1)
		$(target))
endef

# Rule: <Annotation> ::= '@' <Reference> <AnnotationInitializer>
# Args: 1..3 - Symbols in the RHS.
define $(gold_grammar)_produce-Annotation_At
	$(for annotation <- $(new MyAnnotation),
		$(set annotation->type_link,$2)
		$(set annotation->bindings,$3)
		$(annotation))
endef

# Rule: <AnnotationInitializer> ::= '(' <AnnotationParametersList> ')'
# Args: 1..3 - Symbols in the RHS.
$(gold_grammar)_produce-AnnotationInitializer_LParan_RParan = $2

# Rule: <AnnotationInitializer> ::= '(' <Value> ')'
# Args: 1..3 - Symbols in the RHS.
define $(gold_grammar)_produce-AnnotationInitializer_LParan_RParan2
	$(for binding<-$(new MyOptionBinding),
		$(set binding->option_link,$(new ELink,value,$(gold_location)))
		$(set binding->optionValue,$2)
		$(binding))
endef

# Rule: <AnnotationParametersList> ::= <AnnotationParameter> ',' <AnnotationParametersList>
# Args: 1..3 - Symbols in the RHS.
$(gold_grammar)_produce-AnnotationParametersList_Comma = $1 $3

# Rule: <AnnotationParameter> ::= <SimpleReference> '=' <Value>
# Args: 1..3 - Symbols in the RHS.
define $(gold_grammar)_produce-AnnotationParameter_Eq
	$(for binding<-$(new MyOptionBinding),
		$(set binding->option_link,$1)
		$(set binding->optionValue,$3)
		$(binding))
endef

# Rule: <Interface> ::= interface Identifier <SuperInterfaces> '{' <Features> '}'
# Args: 1..6 - Symbols in the RHS.
define $(gold_grammar)_produce-Interface_interface_Identifier_LBrace_RBrace
	$(for interface <- $(new MyInterface),
		$(set interface->name,$2)
		$(set interface->features,$5)
		$(interface))
endef

# Rule: <SuperInterfaces> ::= extends <ReferenceList>
# Args: 1..2 - Symbols in the RHS.
define $(gold_grammar)_produce-SuperInterfaces_extends
	$(gold_default_produce)# TODO Auto-generated stub!
endef

# Rule: <Features> ::= <AnnotatedFeature> <Features>
# Args: 1..2 - Symbols in the RHS.
define $(gold_grammar)_produce-Features
	$(gold_default_produce)# TODO Auto-generated stub!
endef

# Rule: <AnnotatedFeature> ::= <Annotations> <Feature>
# Args: 1..2 - Symbols in the RHS.
define $(gold_grammar)_produce-AnnotatedFeature
	$(gold_default_produce)# TODO Auto-generated stub!
endef

# Rule: <Feature> ::= feature Identifier <SuperFeatures>
# Args: 1..3 - Symbols in the RHS.
define $(gold_grammar)_produce-Feature_feature_Identifier
	$(for feature <- $(new MyFeature),
		$(set feature->name,$2)
		$(set feature->superFeatures_links,$3)
		$(feature))
endef

# Rule: <SuperFeatures> ::= extends <ReferenceList>
# Args: 1..2 - Symbols in the RHS.
$(gold_grammar)_produce-SuperFeatures_extends = $2

# Rule: <ModuleType> ::= <ModuleModifiers> module Identifier <SuperModule> '{' <ModuleMembers> '}'
# Args: 1..7 - Symbols in the RHS.
define $(gold_grammar)_produce-ModuleType_module_Identifier_LBrace_RBrace
	$(foreach module,$(new MyModuleType),
		$(set module->name,$3)
		$(set module->origin,$(call gold_location_of,3))

		$(set module->modifiers,$1)

		$(set module->superType_link,$4)

		$(silent-foreach attr, \
				sourcesMembers \
				optionsMembers \
				dependsMembers \
				requiresMembers \
				providesMembers,
				$(set module->$(attr),
					$(filter-patsubst $(attr)/%,%,$6)))

		$(module)
	)
endef

# Rule: <ModuleModifiers> ::= <ModuleModifier> <ModuleModifiers>
# Args: 1..2 - Symbols in the RHS.
define $(gold_grammar)_produce-ModuleModifiers
	$(if $(filter $1,$2),
		$(call gold_report_error,
				Duplicate module modifier '$1'),
		$1 \
	)
	$2
endef

# Rule: <SuperModule> ::= extends <Reference>
# Args: 1..2 - Symbols in the RHS.
$(gold_grammar)_produce-SuperModule_extends = $2

# Rule: <AnnotatedModuleMember> ::= <Annotations> <ModuleMember>
# Args: 1..2 - Symbols in the RHS.
define $(gold_grammar)_produce-AnnotatedModuleMember
	$(for target <- $2,
		$(set target->annotations,$1)
		$(target))
endef

# Rule: <ModuleMember> ::= depends <ReferenceList>
$(gold_grammar)_produce-ModuleMember_depends = \
	$(for member <- $(new MyDependsMember),\
		$(set member->modules_links,$2)\
		dependsMembers/$(member))

# Rule: <ModuleMember> ::= provides <ReferenceList>
$(gold_grammar)_produce-ModuleMember_provides = \
	$(for member <- $(new MyProvidesMember),\
		$(set member->features_links,$2)\
		providesMembers/$(member))

# Rule: <ModuleMember> ::= requires <ReferenceList>
$(gold_grammar)_produce-ModuleMember_requires = \
	$(for member <- $(new MyRequiresMember),\
		$(set member->features_links,$2)\
		requiresMembers/$(member))

# Rule: <ModuleMember> ::= source <FilenameList>
$(gold_grammar)_produce-ModuleMember_source = \
	$(for member <- $(new MySourceMember),\
		$(set member->files,$2)\
		sourcesMembers/$(member))

# Rule: <ModuleMember> ::= object <FilenameList>
$(gold_grammar)_produce-ModuleMember_object = \
	$(for member <- $(new MyObjectMember),\
		$(set member->files,$2)\
		objectsMembers/$(member))

# Rule: <ModuleMember> ::= option <Option>
# Args: 1..2 - Symbols in the RHS.
$(gold_grammar)_produce-ModuleMember_option = \
	$(for member <- $(new MyOptionMember),\
		$(set member->options,$2)\
		optionsMembers/$(member))

# Rule: <Option> ::= <OptionType> Identifier <OptionDefaultValue>
# Args: 1..3 - Symbols in the RHS.
define $(gold_grammar)_produce-Option_Identifier
    $(for opt <- $(new My$1Option),
		$(set opt->name,$2)
		$(and $3,
			$(if $(invoke opt->validateValue,$3),
				$(set opt->defaultValue,$3),
				$(call gold_report_error_at,
					$(call gold_location_of,3),
					Option value has wrong type)))

		$(opt))
endef

# Rule: <OptionType> ::= string
# Args: 1..1 - Symbols in the RHS.
$(gold_grammar)_produce-OptionType_string = String

# Rule: <OptionType> ::= number
# Args: 1..1 - Symbols in the RHS.
$(gold_grammar)_produce-OptionType_number = Number

# Rule: <OptionType> ::= boolean
# Args: 1..1 - Symbols in the RHS.
$(gold_grammar)_produce-OptionType_boolean = Boolean

# Rule: <OptionDefaultValue> ::= '=' <Value>
# Args: 1..2 - Symbols in the RHS.
$(gold_grammar)_produce-OptionDefaultValue_Eq = $2

# Rule: <Value> ::= StringLiteral
# Args: 1..1 - Symbols in the RHS.
$(gold_grammar)_produce-Value_StringLiteral = $(new MyStringOptionValue,$1)

# Rule: <Value> ::= NumberLiteral
# Args: 1..1 - Symbols in the RHS.
$(gold_grammar)_produce-Value_NumberLiteral = $(new MyNumberOptionValue,$1)

# Rule: <Value> ::= BooleanLiteral
# Args: 1..1 - Symbols in the RHS.
$(gold_grammar)_produce-Value_BooleanLiteral = $(new MyBooleanOptionValue,$1)

# Rule: <Reference> ::= <QualifiedName>
# Args: 1..1 - Symbols in the RHS.
$(gold_grammar)_produce-Reference = $(new ELink,$1,$(gold_location))

# Rule: <SimpleReference> ::= Identifier
# Args: 1..1 - Symbols in the RHS.
$(gold_grammar)_produce-SimpleReference_Identifier = $(new ELink,$1,$(gold_location))

# Rule: <Filename> ::= StringLiteral
# Args: 1..1 - Symbols in the RHS.
define $(gold_grammar)_produce-Filename_StringLiteral
	$(for file <- $(new MyFileName),
		$(set file->fileName,$1)
				$(file))
endef

# Rule: <ReferenceList> ::= <Reference> ',' <ReferenceList>
# Args: 1..3 - Symbols in the RHS.
$(gold_grammar)_produce-ReferenceList_Comma = $1 $3

# Rule: <FilenameList> ::= <Filename> ',' <FilenameList>
# Args: 1..3 - Symbols in the RHS.
$(gold_grammar)_produce-FilenameList_Comma = $1 $3

# <QualifiedName> ::= Identifier '.' <QualifiedName>
$(gold_grammar)_produce-QualifiedName_Identifier_Dot         = $1.$3
# <QualifiedNameWithWildcard> ::= <QualifiedName> '.*'
$(gold_grammar)_produce-QualifiedNameWithWildcard_DotTimes   = $1.*


