#include <sstream>
#include <iostream>
#include <cstring>
#include <string>

#ifdef _WIN32
	#include <windows.h>
#endif

#include "Plugin.hpp"

#ifdef LUA_DEFINE
# undef LUA_DEFINE
#endif
#define LUA_DEFINE(name) Define(#name, [](lua_State *L) -> int

#ifdef _WIN32
	typedef UINT(CALLBACK* JVMDLLFunction)(JavaVM**, void**, JavaVMInitArgs*);
#endif

int Plugin::CreateJava(std::string classPath)
{
	int id = 0;
	while (this->jvms[id] != nullptr)
		id++;

	#ifdef _WIN32
		char inputJdkDll[] = "%JAVA_HOME%\\jre\\bin\\server\\jvm.dll";
		TCHAR outputJdkDll[32000];
		DWORD jdkResult = ExpandEnvironmentStrings((LPCTSTR)inputJdkDll, outputJdkDll, sizeof(outputJdkDll) / sizeof(*outputJdkDll));

		char inputJreDll[] = "%JAVA_HOME%\\bin\\server\\jvm.dll";
		TCHAR outputJreDll[32000];
		DWORD jreResult = ExpandEnvironmentStrings((LPCTSTR)inputJreDll, outputJreDll, sizeof(outputJreDll) / sizeof(*outputJreDll));

		if (!jdkResult && !jreResult) {
			Onset::Plugin::Get()->Log("Failed to find JDK/JRE jvm.dll, please ensure Java 8 is installed.");
			return -1;
		}

		HINSTANCE jvmDLL = LoadLibrary(outputJdkDll);
		if (!jvmDLL) {
			jvmDLL = LoadLibrary(outputJreDll);

			if (!jvmDLL) {
				Onset::Plugin::Get()->Log("Failed to find JDK/JRE jvm.dll, please ensure Java 8 is installed.");
				return -1;
			}
		}

		JVMDLLFunction createJavaVMFunction = (JVMDLLFunction)GetProcAddress(jvmDLL, "JNI_CreateJavaVM");
		if (!createJavaVMFunction) {
			Onset::Plugin::Get()->Log("Failed to find JDK/JRE jvm.dll, please ensure Java 8 is installed.");
			return -1;
		}
	#endif

	static std::stringstream optionString;
	optionString << "-Djava.class.path=" << classPath;

	JavaVMInitArgs vm_args;
	auto* options = new JavaVMOption[1];
	char cpoptionString[200] = "";
	std::strcpy(cpoptionString, optionString.str().c_str());
	
	options[0].optionString = cpoptionString;
	vm_args.version = JNI_VERSION_1_8;
	vm_args.nOptions = 1;
	vm_args.options = options;
	vm_args.ignoreUnrecognized = false;

	#ifdef __linux__ 
		JNI_CreateJavaVM(&this->jvms[id], (void**)&this->jenvs[id], &vm_args);
	#elif _WIN32
		createJavaVMFunction(&this->jvms[id], (void**)&this->jenvs[id], &vm_args);
	#endif

	return id + 1;
}

void Plugin::DestroyJava(int id)
{
	this->jvms[id - 1]->DestroyJavaVM();
	this->jvms[id - 1] = nullptr;
	this->jenvs[id - 1] = nullptr;
}

JavaVM* Plugin::GetJavaVM(int id)
{
	return this->jvms[id - 1];
}

JNIEnv* Plugin::GetJavaEnv(int id)
{
	return this->jenvs[id - 1];
}

jobject Plugin::ToJavaObject(JNIEnv* jenv, Lua::LuaValue value)
{
	switch (value.GetType())
	{
		case Lua::LuaValue::Type::STRING:
			{
				return (jobject)jenv->NewStringUTF(value.GetValue<std::string>().c_str());
			} break;
		case Lua::LuaValue::Type::INTEGER:
			{
				jclass jcls = jenv->FindClass("java/lang/Integer");
				return jenv->NewObject(jcls, jenv->GetMethodID(jcls, "<init>", "(I)V"), value.GetValue<int>());
			} break;
		case Lua::LuaValue::Type::BOOLEAN:
			{
				jclass jcls = jenv->FindClass("java/lang/Boolean");
				return jenv->NewObject(jcls, jenv->GetMethodID(jcls, "<init>", "(Z)V"), value.GetValue<bool>());
			} break;
		case Lua::LuaValue::Type::TABLE:
			{
				jclass jcls = jenv->FindClass("java/util/HashMap");
				jobject jmap = jenv->NewObject(jcls, jenv->GetMethodID(jcls, "<init>", "()V"));
				jmethodID putMethod = jenv->GetMethodID(jcls, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

				Lua::LuaTable_t table = value.GetValue<Lua::LuaTable_t>();
				table->ForEach([jenv, jmap, putMethod](Lua::LuaValue k, Lua::LuaValue v) {
					jenv->CallObjectMethod(jmap, putMethod, Plugin::Get()->ToJavaObject(jenv, k), Plugin::Get()->ToJavaObject(jenv, v));
				});

				return jmap;
			} break;
		case Lua::LuaValue::Type::NIL:
		case Lua::LuaValue::Type::INVALID:
			break;
		default:
			break;
	}

	return NULL;
}

Lua::LuaValue Plugin::ToLuaValue(JNIEnv* jenv, jobject object)
{
	jclass jcls = jenv->GetObjectClass(object);

	if (jenv->IsInstanceOf(object, jenv->FindClass("java/lang/String"))) {
		jstring element = (jstring)object;
		const char* pchars = jenv->GetStringUTFChars(element, nullptr);

		Lua::LuaValue value(pchars);

		jenv->ReleaseStringUTFChars(element, pchars);
		jenv->DeleteLocalRef(element);

		return value;
	}
	else if (jenv->IsInstanceOf(object, jenv->FindClass("java/lang/Integer"))) {
		jmethodID intValueMethod = jenv->GetMethodID(jcls, "intValue", "()I");
		jint result = jenv->CallIntMethod(object, intValueMethod);

		Lua::LuaValue value(result);
		return value;
	}
	else if (jenv->IsInstanceOf(object, jenv->FindClass("java/lang/Boolean"))) {
		jmethodID boolValueMethod = jenv->GetMethodID(jcls, "booleanValue", "()Z");
		jboolean result = jenv->CallBooleanMethod(object, boolValueMethod);

		Lua::LuaValue value(result);
		return value;
	}
	else if (jenv->IsInstanceOf(object, jenv->FindClass("java/util/List"))) {
		jmethodID sizeMethod = jenv->GetMethodID(jcls, "size", "()I");
		jmethodID getMethod = jenv->GetMethodID(jcls, "get", "(I)Ljava/lang/Object;");
		jint len = jenv->CallIntMethod(object, sizeMethod);

		Lua::LuaTable_t table(new Lua::LuaTable);
		for (jint i = 0; i < len; i++) {
			jobject arrayElement = jenv->CallObjectMethod(object, getMethod, i);
			table->Add(i + 1, Plugin::Get()->ToLuaValue(jenv, arrayElement));
		}

		Lua::LuaValue value(table);
		return value;
	}
	else if (jenv->IsInstanceOf(object, jenv->FindClass("java/util/Map"))) {
		jmethodID getMethod = jenv->GetMethodID(jcls, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");

		jmethodID keySetMethod = jenv->GetMethodID(jcls, "keySet", "()Ljava/util/Set;");
		jobject keySet = jenv->CallObjectMethod(object, keySetMethod);

		jmethodID keySetToArrayMethod = jenv->GetMethodID(jenv->GetObjectClass(keySet), "toArray", "()[Ljava/lang/Object;");
		jobjectArray keyArray = (jobjectArray)jenv->CallObjectMethod(keySet, keySetToArrayMethod);
		int arraySize = jenv->GetArrayLength(keyArray);

		Lua::LuaTable_t table(new Lua::LuaTable);
		for (int i = 0; i < arraySize; i++)
		{
			jobject key = jenv->GetObjectArrayElement(keyArray, i);
			jobject value = jenv->CallObjectMethod(object, getMethod, key);

			table->Add(Plugin::Get()->ToLuaValue(jenv, key), Plugin::Get()->ToLuaValue(jenv, value));
		}

		Lua::LuaValue value(table);
		return value;
	}

	return NULL;
}

void CallEvent(JNIEnv* jenv, jclass jcl, jstring event, jobject argsList) {
	(void) jcl;

	const char* eventStr = jenv->GetStringUTFChars(event, nullptr);
	auto args = new Lua::LuaArgs_t();

	if (jenv->IsInstanceOf(argsList, jenv->FindClass("java/util/List"))) {
		jclass argsCls = jenv->GetObjectClass(argsList);
		jmethodID sizeMethod = jenv->GetMethodID(argsCls, "size", "()I");
		jmethodID getMethod = jenv->GetMethodID(argsCls, "get", "(I)Ljava/lang/Object;");
		jint len = jenv->CallIntMethod(argsList, sizeMethod);

		for (jint i = 0; i < len; i++) {
			jobject arrayElement = jenv->CallObjectMethod(argsList, getMethod, i);
			args->push_back(Plugin::Get()->ToLuaValue(jenv, arrayElement));
		}
	}

	Onset::Plugin::Get()->CallEvent(eventStr, args);

	jenv->ReleaseStringUTFChars(event, eventStr);
	jenv->DeleteLocalRef(event);
}

Plugin::Plugin()
{
	for (int i = 0; i < 30; i++)
		this->jvms[i] = nullptr;

	LUA_DEFINE(CreateJava)
	{
		std::string classPath;
		Lua::ParseArguments(L, classPath);
		int id = Plugin::Get()->CreateJava(classPath);

		if (id < 0) return 0;
		return Lua::ReturnValues(L, id);
	});

	LUA_DEFINE(DestroyJava)
	{
		int id;
		Lua::ParseArguments(L, id);
		Plugin::Get()->DestroyJava(id);

		return 1;
	});

	LUA_DEFINE(LinkJavaAdapter)
	{
		int id;
		Lua::ParseArguments(L, id);

		if (!Plugin::Get()->GetJavaEnv(id)) return 0;
		JNIEnv* jenv = Plugin::Get()->GetJavaEnv(id);

		Lua::LuaArgs_t arg_list;
		Lua::ParseArguments(L, arg_list);

		int arg_size = static_cast<int>(arg_list.size());
		if (arg_size < 2) return 0;

		std::string className = arg_list[1].GetValue<std::string>();
		jclass clazz = jenv->FindClass(className.c_str());
		if (clazz == nullptr) return 0;

		JNINativeMethod methods[] = {
			{(char*)"callEvent", (char*)"(Ljava/lang/String;Ljava/util/List;)V", (void*)CallEvent },
		};

		jenv->RegisterNatives(clazz, methods, 1);

		Lua::ReturnValues(L, 1);
		return 1;
	});

	LUA_DEFINE(CallJavaStaticMethod)
	{
		int id;
		Lua::ParseArguments(L, id);

		if (!Plugin::Get()->GetJavaEnv(id)) return 0;
		JNIEnv* jenv = Plugin::Get()->GetJavaEnv(id);

		Lua::LuaArgs_t arg_list;
		Lua::ParseArguments(L, arg_list);

		int arg_size = static_cast<int>(arg_list.size());
		if (arg_size < 4) return 0;

		std::string className = arg_list[1].GetValue<std::string>();
		std::string methodName = arg_list[2].GetValue<std::string>();

		std::string signature = arg_list[3].GetValue<std::string>();
		size_t spos = signature.find(")");
		std::string returnSignature = signature.substr(spos + 1, signature.length() - spos);

		jobject* params = new jobject[arg_size - 4];
		for (int i = 4; i < arg_size; i++) {
			auto const& value = arg_list[i];
			params[i - 4] = Plugin::Get()->ToJavaObject(jenv, value);
		}

		jclass clazz = jenv->FindClass(className.c_str());
		if (clazz == nullptr) return 0;

		jmethodID methodID = jenv->GetStaticMethodID(clazz, methodName.c_str(), signature.c_str());
		if (methodID == nullptr) return 0;

		jobject returnValue = nullptr;
		if (!returnSignature.compare("V")) {
			switch (arg_size - 4) {
				case 0:
					jenv->CallStaticVoidMethod(clazz, methodID);
					break;
				case 1:
					jenv->CallStaticVoidMethod(clazz, methodID, params[0]);
					break;
				case 2:
					jenv->CallStaticVoidMethod(clazz, methodID, params[0], params[1]);
					break;
				case 3:
					jenv->CallStaticVoidMethod(clazz, methodID, params[0], params[1], params[2]);
					break;
				case 4:
					jenv->CallStaticVoidMethod(clazz, methodID, params[0], params[1], params[2], params[3]);
					break;
				case 5:
					jenv->CallStaticVoidMethod(clazz, methodID, params[0], params[1], params[2], params[3], params[4]);
					break;
				case 6:
					jenv->CallStaticVoidMethod(clazz, methodID, params[0], params[1], params[2], params[3], params[4], params[5]);
					break;
				case 7:
					jenv->CallStaticVoidMethod(clazz, methodID, params[0], params[1], params[2], params[3], params[4], params[5], params[6]);
					break;
				case 8:
					jenv->CallStaticVoidMethod(clazz, methodID, params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7]);
					break;
				case 9:
					jenv->CallStaticVoidMethod(clazz, methodID, params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7], params[8]);
					break;
				case 10:
					jenv->CallStaticVoidMethod(clazz, methodID, params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7], params[8], params[9]);
					break;
				default:
					Onset::Plugin::Get()->Log("Too many parameters for CallJavaStaticMethod, 10 max.");
					break;
			}
		}
		else {
			switch (arg_size - 4) {
			case 0:
				returnValue = jenv->CallStaticObjectMethod(clazz, methodID);
				break;
			case 1:
				returnValue = jenv->CallStaticObjectMethod(clazz, methodID, params[0]);
				break;
			case 2:
				returnValue = jenv->CallStaticObjectMethod(clazz, methodID, params[0], params[1]);
				break;
			case 3:
				returnValue = jenv->CallStaticObjectMethod(clazz, methodID, params[0], params[1], params[2]);
				break;
			case 4:
				returnValue = jenv->CallStaticObjectMethod(clazz, methodID, params[0], params[1], params[2], params[3]);
				break;
			case 5:
				returnValue = jenv->CallStaticObjectMethod(clazz, methodID, params[0], params[1], params[2], params[3], params[4]);
				break;
			case 6:
				returnValue = jenv->CallStaticObjectMethod(clazz, methodID, params[0], params[1], params[2], params[3], params[4], params[5]);
				break;
			case 7:
				returnValue = jenv->CallStaticObjectMethod(clazz, methodID, params[0], params[1], params[2], params[3], params[4], params[5], params[6]);
				break;
			case 8:
				returnValue = jenv->CallStaticObjectMethod(clazz, methodID, params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7]);
				break;
			case 9:
				returnValue = jenv->CallStaticObjectMethod(clazz, methodID, params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7], params[8]);
				break;
			case 10:
				returnValue = jenv->CallStaticObjectMethod(clazz, methodID, params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7], params[8], params[9]);
				break;
			default:
				Onset::Plugin::Get()->Log("Too many parameters for CallJavaStaticMethod, 10 max.");
				break;
			}
		}

		if (returnValue != nullptr) {
			Lua::LuaValue value = Plugin::Get()->ToLuaValue(jenv, returnValue);

			if (!(value == NULL)) {
				switch (value.GetType())
				{
					case Lua::LuaValue::Type::STRING:
						Lua::ReturnValues(L, value.GetValue<std::string>().c_str());
						break;
					case Lua::LuaValue::Type::INTEGER:
						Lua::ReturnValues(L, value.GetValue<int>());
						break;
					case Lua::LuaValue::Type::BOOLEAN:
						Lua::ReturnValues(L, value.GetValue<bool>());
						break;
					case Lua::LuaValue::Type::TABLE:
						Lua::ReturnValues(L, value.GetValue<Lua::LuaTable_t>());
						break;
					default:
						break;
				}
			}
		}
		else {
			Lua::ReturnValues(L, 1);
		}

		return 1;
	});
}
