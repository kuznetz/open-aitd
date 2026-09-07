@src/engine/docs/arch/common.md
@src/engine/resources
Create compact architectural C++ documentation for src/engine/resources directory for LLM indexing. 
For each .cpp and .h file, briefly describe:
1. Write full path in header
2. List the names of all classes, structures, and enumerations declared in the file with a brief (1–2 sentence) description of its purpose at the architectural level. Do not list methods, fields, or internal members.
3. Role in the system – additional high-level responsibility that isn’t already conveyed by the class descriptions (so the “role” section will only appear if something significant remains).
4. Describe the relationships between key modules (both internal dependencies within this directory and external dependencies on other engine modules), without diagrams.
The result must be in Markdown format, understandable for an LLM, in English.
----
@src/engine/docs/arch
Create a compact file table of contents for all MD files for LLM, list the components inside these files, and provide a brief description of each MD file (1–3 sentences). Include the full path to the MD file in the header.
----
@src/engine/docs/arch/index.md
@src/engine/main.cpp
If necessary, first use the documentation from `src/engine/docs/arch/` and then the source code.