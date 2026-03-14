# Code Practices

## Summary

This page documents the coding conventions enforced in Katalog's C++ source code.

---

## Diagnostic Output

This section describes the use of qDebug / qWarning / qInfo messages.

Expected limitations or possible errors are usually reported to users via Katalog's UI.
Other less probable failures or what would result from a direct modification of Collection data (loss of consistency or validity) or direct modification of the database structure will not be reported via the UI (too much code and translation effort).

Qt provides four macros, each with a distinct visibility and semantic meaning:

| Macro         | Always visible  | Meaning                  |
|---------------|:---------------:|--------------------------|
| `qDebug()`    | No — can be compiled out or filtered at runtime | Developer messages used during active development only. <br/>**Do not use in committed code.** |
| `qInfo()`     | Yes             | Normal informational output intended for the user: CLI results, progress summaries |
| `qWarning()`  | Yes             | An operation failed or hit an unexpected state — user should be aware |
| `qCritical()` | Yes             | serious failure that prevents correct operation  <br/> **NOT USED IN Katalog at the moment**|

### Message format

All kept messages must follow this format:

```
qWarning() << "WARNING: ClassName::methodName - description of what failed:" << value;
```

Rules:
- **`"WARNING: "`** prefix is mandatory — it makes the message immediately visible in a wall of terminal output, even when not captured by the system journal
- **`ClassName::methodName`** is mandatory — it makes the message self-contained when read out of context (log file, support report)
- Include the relevant value (path, ID, error string) so the message is actionable without access to the source code

### Special case: commandline.cpp

`commandline.cpp` produces output intentionally directed at users running Katalog from the terminal (update results, catalog lists, error summaries). Use `qInfo()` for all of it — it maps to stdout and is never suppressed:

```cpp
// commandline.cpp — qInfo() for all user-facing CLI output
qInfo() << "Catalog update failed:" << deviceID;
qInfo() << "Failed catalogs:";
for (const QString &name : failedCatalogs)
    qInfo() << " -" << name;
```

