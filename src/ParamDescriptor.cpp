//
// Created by dsporykhin on 30.04.20.
//

#include "ParamDescriptor.h"

ParamDescriptor::ParamDescriptor(String paramName, ParamType paramType, float minValue, float maxValue,
                                 void *valueReferenceForRead, void *valueReferenceForWrite) {
    this->paramName = paramName;
    this->paramType = paramType;
    this->minValue = minValue;
    this->maxValue = maxValue;
    this->valueReferenceForRead = valueReferenceForRead;
    this->valueReferenceForWrite = valueReferenceForWrite;
    this->arraySize = 0;
    this->byte_data_length = 0;
}

ParamDescriptor::ParamDescriptor(String paramName, ParamType paramType, float minValue, float maxValue, int arraySize,
                                 void *valueReferenceForRead, void *valueReferenceForWrite) {
    this->paramName = paramName;
    this->paramType = paramType;
    this->minValue = minValue;
    this->maxValue = maxValue;
    this->valueReferenceForRead = valueReferenceForRead;
    this->valueReferenceForWrite = valueReferenceForWrite;
    this->arraySize = arraySize;
    this->byte_data_length = 0;
}
ParamDescriptor::ParamDescriptor(String paramName, int byte_data_length,
                                 void *valueReferenceForRead, void *valueReferenceForWrite) {
    this->paramName = paramName;
    this->paramType = HEX_BYTES;
    this->minValue = 0;
    this->maxValue = 0;
    this->valueReferenceForRead = valueReferenceForRead;
    this->valueReferenceForWrite = valueReferenceForWrite;
    this->arraySize = 0;
    this->byte_data_length = byte_data_length;
}
