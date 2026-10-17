#pragma once
typedef struct objc_object *id;
@interface NSString { id isa; } - (const char *)UTF8String; @end
@interface NSObjectStub { id isa; } - (NSString *)description; @end
