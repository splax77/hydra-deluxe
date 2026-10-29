<#
.SYNOPSIS
Read the .NET metadata of a Mono build's game assemblies and write what it
holds as JSON, for mono_facts.py.

.DESCRIPTION
mono_facts.py runs this script and owns everything done with its output: the
class kind, the base class rule, which fields count, and the facts.json shape.
This script only reads. It uses System.Reflection.Metadata, which ships with
PowerShell 7, so nothing is installed or fetched. The C# below decodes field
and method signatures into type names written the way Il2CppDumper's dump.cs
writes them, so the names line up with a facts.json made from an IL2CPP build.

.EXAMPLE
pwsh -NoProfile -File tools/ch_mcp/mono_reader.ps1 -Out reader.json A.dll B.dll
#>
param(
    [Parameter(Mandatory = $true)] [string] $Out,
    [Parameter(Mandatory = $true, ValueFromRemainingArguments = $true)] [string[]] $Assemblies
)

$ErrorActionPreference = 'Stop'

# The type name carries a version number because a PowerShell host keeps an
# Add-Type type for its whole life; bump it whenever the C# changes.
$source = @'
using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.IO;
using System.Reflection;
using System.Reflection.Metadata;
using System.Reflection.Metadata.Ecma335;
using System.Reflection.PortableExecutable;
using System.Text.Json;

public sealed class ChMcpMonoContextV1
{
    public readonly string[] TypeParams;
    public readonly string[] MethodParams;
    public ChMcpMonoContextV1(string[] typeParams, string[] methodParams)
    {
        TypeParams = typeParams;
        MethodParams = methodParams;
    }
}

public sealed class ChMcpMonoReaderV1 : ISignatureTypeProvider<string, ChMcpMonoContextV1>
{
    static readonly string[] None = new string[0];

    // Il2CppDumper drops the `N arity suffix metadata adds to generic names.
    static string Strip(string name)
    {
        int tick = name.IndexOf('`');
        return tick < 0 ? name : name.Substring(0, tick);
    }

    public string GetPrimitiveType(PrimitiveTypeCode code)
    {
        switch (code)
        {
            case PrimitiveTypeCode.Boolean: return "bool";
            case PrimitiveTypeCode.Char: return "char";
            case PrimitiveTypeCode.SByte: return "sbyte";
            case PrimitiveTypeCode.Byte: return "byte";
            case PrimitiveTypeCode.Int16: return "short";
            case PrimitiveTypeCode.UInt16: return "ushort";
            case PrimitiveTypeCode.Int32: return "int";
            case PrimitiveTypeCode.UInt32: return "uint";
            case PrimitiveTypeCode.Int64: return "long";
            case PrimitiveTypeCode.UInt64: return "ulong";
            case PrimitiveTypeCode.Single: return "float";
            case PrimitiveTypeCode.Double: return "double";
            case PrimitiveTypeCode.String: return "string";
            case PrimitiveTypeCode.Object: return "object";
            case PrimitiveTypeCode.Void: return "void";
            default: return code.ToString();
        }
    }

    public string GetTypeFromDefinition(MetadataReader reader, TypeDefinitionHandle handle, byte rawTypeKind)
    {
        TypeDefinition td = reader.GetTypeDefinition(handle);
        string name = Strip(reader.GetString(td.Name));
        TypeDefinitionHandle outer = td.GetDeclaringType();
        return outer.IsNil ? name : GetTypeFromDefinition(reader, outer, 0) + "." + name;
    }

    public string GetTypeFromReference(MetadataReader reader, TypeReferenceHandle handle, byte rawTypeKind)
    {
        TypeReference tr = reader.GetTypeReference(handle);
        string name = Strip(reader.GetString(tr.Name));
        if (tr.ResolutionScope.Kind == HandleKind.TypeReference)
            return GetTypeFromReference(reader, (TypeReferenceHandle)tr.ResolutionScope, 0) + "." + name;
        return name;
    }

    public string GetTypeFromSpecification(MetadataReader reader, ChMcpMonoContextV1 context, TypeSpecificationHandle handle, byte rawTypeKind)
    {
        return reader.GetTypeSpecification(handle).DecodeSignature(this, context);
    }

    public string GetSZArrayType(string elementType) { return elementType + "[]"; }
    public string GetArrayType(string elementType, ArrayShape shape) { return elementType + "[" + new string(',', shape.Rank - 1) + "]"; }
    public string GetByReferenceType(string elementType) { return "ref " + elementType; }
    public string GetPointerType(string elementType) { return elementType + "*"; }
    public string GetPinnedType(string elementType) { return elementType; }
    public string GetModifiedType(string modifier, string unmodifiedType, bool isRequired) { return unmodifiedType; }
    public string GetFunctionPointerType(MethodSignature<string> signature) { return "IntPtr"; }
    public string GetGenericInstantiation(string genericType, ImmutableArray<string> typeArguments)
    {
        return genericType + "<" + string.Join(", ", typeArguments) + ">";
    }
    public string GetGenericTypeParameter(ChMcpMonoContextV1 context, int index) { return context.TypeParams[index]; }
    public string GetGenericMethodParameter(ChMcpMonoContextV1 context, int index) { return context.MethodParams[index]; }

    static string[] ParamNames(MetadataReader reader, GenericParameterHandleCollection handles)
    {
        if (handles.Count == 0) return None;
        var names = new string[handles.Count];
        int i = 0;
        foreach (GenericParameterHandle h in handles)
            names[i++] = reader.GetString(reader.GetGenericParameter(h).Name);
        return names;
    }

    // The header name as dump.cs prints it: the outer classes' bare names,
    // then this class's bare name with its generic parameters.
    string HeaderName(MetadataReader reader, TypeDefinitionHandle handle, string[] typeParams)
    {
        string name = GetTypeFromDefinition(reader, handle, 0);
        return typeParams.Length == 0 ? name : name + "<" + string.Join(", ", typeParams) + ">";
    }

    string TypeName(MetadataReader reader, EntityHandle handle, ChMcpMonoContextV1 context)
    {
        switch (handle.Kind)
        {
            case HandleKind.TypeDefinition: return GetTypeFromDefinition(reader, (TypeDefinitionHandle)handle, 0);
            case HandleKind.TypeReference: return GetTypeFromReference(reader, (TypeReferenceHandle)handle, 0);
            case HandleKind.TypeSpecification: return GetTypeFromSpecification(reader, context, (TypeSpecificationHandle)handle, 0);
            default: throw new InvalidOperationException("unexpected type handle kind " + handle.Kind);
        }
    }

    // Namespace-qualified name of a base type, so the caller can tell
    // System.Enum and System.ValueType from a game class of the same name.
    static string FullName(MetadataReader reader, EntityHandle handle)
    {
        if (handle.Kind == HandleKind.TypeReference)
        {
            TypeReference tr = reader.GetTypeReference((TypeReferenceHandle)handle);
            string ns = reader.GetString(tr.Namespace);
            return ns.Length == 0 ? reader.GetString(tr.Name) : ns + "." + reader.GetString(tr.Name);
        }
        if (handle.Kind == HandleKind.TypeDefinition)
        {
            TypeDefinition td = reader.GetTypeDefinition((TypeDefinitionHandle)handle);
            string ns = reader.GetString(td.Namespace);
            return ns.Length == 0 ? reader.GetString(td.Name) : ns + "." + reader.GetString(td.Name);
        }
        return null;
    }

    // dump.cs writes a by-reference parameter as out, in or ref from its flags.
    static string ParamType(string decoded, ParameterAttributes flags)
    {
        if (!decoded.StartsWith("ref ")) return decoded;
        bool isOut = (flags & ParameterAttributes.Out) != 0;
        bool isIn = (flags & ParameterAttributes.In) != 0;
        if (isOut && !isIn) return "out " + decoded.Substring(4);
        if (isIn && !isOut) return "in " + decoded.Substring(4);
        return decoded;
    }

    Dictionary<string, object> ReadType(MetadataReader reader, TypeDefinitionHandle handle, string assembly)
    {
        TypeDefinition td = reader.GetTypeDefinition(handle);
        string[] typeParams = ParamNames(reader, td.GetGenericParameters());
        var typeContext = new ChMcpMonoContextV1(typeParams, None);

        var fields = new List<object>();
        foreach (FieldDefinitionHandle fh in td.GetFields())
        {
            FieldDefinition fd = reader.GetFieldDefinition(fh);
            fields.Add(new Dictionary<string, object> {
                { "name", reader.GetString(fd.Name) },
                { "type", fd.DecodeSignature(this, typeContext) },
                { "static", (fd.Attributes & FieldAttributes.Static) != 0 },
                { "literal", (fd.Attributes & FieldAttributes.Literal) != 0 },
            });
        }

        var methods = new List<object>();
        foreach (MethodDefinitionHandle mh in td.GetMethods())
        {
            MethodDefinition md = reader.GetMethodDefinition(mh);
            string[] methodParams = ParamNames(reader, md.GetGenericParameters());
            MethodSignature<string> sig = md.DecodeSignature(this, new ChMcpMonoContextV1(typeParams, methodParams));
            var flags = new ParameterAttributes[sig.ParameterTypes.Length];
            foreach (ParameterHandle ph in md.GetParameters())
            {
                Parameter p = reader.GetParameter(ph);
                if (p.SequenceNumber >= 1 && p.SequenceNumber <= flags.Length)
                    flags[p.SequenceNumber - 1] = p.Attributes;
            }
            var ps = new List<object>();
            for (int i = 0; i < sig.ParameterTypes.Length; i++)
                ps.Add(ParamType(sig.ParameterTypes[i], flags[i]));
            string name = reader.GetString(md.Name);
            if (methodParams.Length > 0) name += "<" + string.Join(", ", methodParams) + ">";
            methods.Add(new Dictionary<string, object> {
                { "name", name },
                { "return", sig.ReturnType },
                { "params", ps },
                { "static", (md.Attributes & MethodAttributes.Static) != 0 },
            });
        }

        var interfaces = new List<object>();
        foreach (InterfaceImplementationHandle ih in td.GetInterfaceImplementations())
            interfaces.Add(TypeName(reader, reader.GetInterfaceImplementation(ih).Interface, typeContext));

        EntityHandle baseHandle = td.BaseType;
        return new Dictionary<string, object> {
            { "assembly", assembly },
            { "row", MetadataTokens.GetRowNumber(handle) },
            { "namespace", reader.GetString(td.Namespace) },
            { "name", HeaderName(reader, handle, typeParams) },
            { "is_interface", (td.Attributes & TypeAttributes.Interface) != 0 },
            { "base", baseHandle.IsNil ? null : TypeName(reader, baseHandle, typeContext) },
            { "base_full", baseHandle.IsNil ? null : FullName(reader, baseHandle) },
            { "interfaces", interfaces },
            { "fields", fields },
            { "methods", methods },
        };
    }

    public static string Read(string[] paths)
    {
        var assemblies = new List<object>();
        foreach (string path in paths)
        {
            using (var stream = File.OpenRead(path))
            using (var pe = new PEReader(stream))
            {
                MetadataReader reader = pe.GetMetadataReader();
                var provider = new ChMcpMonoReaderV1();
                string file = Path.GetFileName(path);
                var types = new List<object>();
                foreach (TypeDefinitionHandle h in reader.TypeDefinitions)
                    types.Add(provider.ReadType(reader, h, file));
                assemblies.Add(new Dictionary<string, object> { { "file", file }, { "types", types } });
            }
        }
        return JsonSerializer.Serialize(new Dictionary<string, object> { { "assemblies", assemblies } });
    }
}
'@

if (-not ('ChMcpMonoReaderV1' -as [type])) {
    Add-Type -TypeDefinition $source -Language CSharp
}

$json = [ChMcpMonoReaderV1]::Read([string[]] $Assemblies)
[System.IO.File]::WriteAllText($Out, $json, [System.Text.UTF8Encoding]::new($false))
