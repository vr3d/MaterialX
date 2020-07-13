//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTMESSAGE_H
#define MATERIALX_RTMESSAGE_H

/// @file
/// Classes for notification of data model changes.

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtTypeInfo.h>
#include <MaterialXRuntime/RtAttribute.h>
#include <MaterialXRuntime/RtRelationship.h>

namespace MaterialX
{

class RtAttrIterator;
class RtRelationshipIterator;
class RtPrimIterator;
class RtSchemaBase;

/// Type for storing callback IDs.
using RtCallbackId = size_t;

/// Function type for callback notifying when a prim has been created.
using RtCreatePrimCallbackFunc = std::function<void(RtStagePtr stage, const RtPrim& prim, void* userData)>;

/// Function type for callback notifying when a prim is about to be removed.
using RtRemovePrimCallbackFunc = std::function<void(RtStagePtr stage, const RtPrim& prim, void* userData)>;

/// Function type for callback notifying when a prim is about to be renamed.
using RtRenamePrimCallbackFunc = std::function<void(RtStagePtr stage, const RtPrim& prim, const RtToken& newName, void* userData)>;

/// Function type for callback notifying when a prim is about to be reparented.
using RtReparentPrimCallbackFunc = std::function<void(RtStagePtr stage, const RtPrim& prim, const RtPath& newPath, void* userData)>;

/// Function type for callback notifying when an attribute valus is set.
using RtSetAttributeCallbackFunc = std::function<void(const RtAttribute& attr, const RtValue& value, void* userData)>;

/// Function type for callback notifying when a connection is made.
using RtMakeConnectionCallbackFunc = std::function<void(const RtOutput& src, const RtInput& dest, void* userData)>;

/// Function type for callback notifying when a connection is broken.
using RtBreakConnectionCallbackFunc = std::function<void(const RtOutput& src, const RtInput& dest, void* userData)>;

/// @class RtMessage
class RtMessage
{
public:
    /// Register a callback to get notified when a prim has been created.
    static RtCallbackId addCreatePrimCallback(RtCreatePrimCallbackFunc callback, void* userData = nullptr);

    /// Register a callback to get notified when a prim is about to be removed.
    static RtCallbackId addRemovePrimCallback(RtRemovePrimCallbackFunc callback, void* userData = nullptr);

    /// Register a callback to get notified when a prim is about to be renamed.
    static RtCallbackId addRenamePrimCallback(RtRenamePrimCallbackFunc callback, void* userData = nullptr);

    /// Register a callback to get notified when a prim is about to be reparented.
    static RtCallbackId addReparentPrimCallback(RtReparentPrimCallbackFunc callback, void* userData = nullptr);

    /// Register a callback to get notified when an attribute value is set.
    static RtCallbackId addSetAttributeCallback(RtSetAttributeCallbackFunc callback, void* userData = nullptr);

    /// Register a callback to get notified when a connection is made.
    static RtCallbackId addMakeConnectionCallback(RtMakeConnectionCallbackFunc callback, void* userData = nullptr);

    /// Register a callback to get notified when a connection is broken.
    static RtCallbackId addBreakConnectionCallback(RtBreakConnectionCallbackFunc callback, void* userData = nullptr);

    /// Remove the callback with given id.
    static void removeCallback(RtCallbackId id);
};

}

#endif