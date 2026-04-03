// SagoMagic.h

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"

/** Main log category used across the project */
DECLARE_LOG_CATEGORY_EXTERN(LogSagoMagic, Log, All);

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
