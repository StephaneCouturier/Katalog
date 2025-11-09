# Progress Reporting
## **Introduction**
The goal of this document is to create consistent, reliable, and maintainable messages to report progress using the Status bar.

Requirements:
* the system shall faciliate rich text formatting (bold, color, italic) per part of the message
* the system shall use a common mechanism to generate progress messages in a consistent manner
* the system shall assemble the message so that text is easy to translate and translations are easy to maintain.

## **STANDARD MESSAGE BUILDER**

### Class
StatusBarMessageBuilder

### Message Template
```cpp
QString template = "%1 | %2 | %3 | %4: %5 of %6 (%7%) | %8: %9 | %10";
```
```
Example:
%1     | %2          | %3                        | %4:        %5 of %6    (%7%)
CREATE | Completed   | Catalog 1 of 1 | MyPhotos | Indexed: 2000 of 2000 (100%)
```

### Component Breakdown

| **Part** | **Field Name** | **All Possible Values** | **Notes** |
|----------|----------------|-------------------------|-----------|
| **1** | **Operation** | • "Create" (→ "CREATE")<br/>• "Search" (→ "SEARCH")<br/>• "Explore" (→ "EXPLORE")<br/>• "Update" (→ "UPDATE") | Translated as normal case,<br/>auto-converted to uppercase by builder |
| **2** | **State** | • "In Progress"<br/>• "Paused"<br/>• "Stopped"<br/>• "Cancelled"<br/>• "Completed" | Current state of operation | * note: cancelled = no data saved (create, update), while stopped = some data saved or displayed (search)
| **3** | **Device Context** | • "Catalog X of Y \| CatalogName"<br/> | Format: "Catalog 1 of 5 \| MyPhotos"<br/>CatalogName displayed in HTML color (#39b2e5 blue)<br/>Shown for all catalog operations (even 1 of 1) |
| **4** | **Process Title** | **Search Operations:**<br/>• "File Types Updated" (populate file type)<br/>• "Loaded" (catalog index loading)<br/>• "Evaluated" (file criteria matching)<br/><br/>**Update Operations:**<br/>• "Loaded" (only for Memory mode and catalogs that include Metadata)<br/>• "Counted"<br/>• "Indexed" (files scanned/updated in catalog)<br/>• "File Types Updated"<br/>• "Metadata Extracted"<br/><br/>**Create Operations:**<br/>• "Counted"<br/>• "Indexed" (files added to new catalog)<br/>• "Metadata Extracted"<br/><br/>**Explore Operations:**<br/>• "File Types Updated"<br/>• "Loaded" (catalog loading) | Process being executed, dynamically set by operation |
| **5** | **Current Count** | Any integer ≥ 0 |  Items processed so far |
| **6** | **Total Count** | Any integer > 0, or 0 (unknown) |  Total items (0 = unknown, shows only current) |
| **7** | **Percent** | 0-100 (integer) | Calculated from Part 5 / Part 6 |
| **8** | **Result Title** | • "Files found"<br/>• "Folders found"<br/>• "Found" (generic) |  Files or folders being counted |
| **9** | **Result Count** | Any integer ≥ 0 |  Result accumulator |
| **10** | **Time to Completion** | Duration string (e.g., "5m 23s", "1h 25m 20s") |  Estimated time to completion<br/>Only used for Extract Metadata process<br/>Updates every batch (~100 files) |
| **11** | **Current Item** | File path string |  Current file being processed<br/>Updates every ~50-100 files (batched) |

* DEV: consider spliting the DeviceContext (%3) in 2: DeviceList & DeviceName
* Note about Cancelled vs Stopped: "Cancelled" will be used for operations that would not have changed data when interrupted (ex: Create, Update), while "Stopped" is used when some data has been modified or generated (ex: "Search").
* DEV: make the hide delay a global parameter (5 seconds at the moment)
 
### Examples for all possible Operations, states, & steps

```
//CREATE
CREATE | In Progress | Catalog 1 of 1 | MyPhotos | Counted: 500
CREATE | In Progress | Catalog 1 of 1 | MyPhotos | Indexed: 200 of 2000 (10%)  | /home/stephane/Document/test.odt
CREATE | Cancelled     | Catalog 1 of 1 | MyPhotos | Indexed: 500 of 2000 (25%)
CREATE | Completed   | Catalog 1 of 1 | MyPhotos | Indexed: 2000 of 2000 (100%)

//SEARCH
SEARCH | In Progress | Catalog 1 of 5 | MyPhotos | Loaded: 100 of 1000 (10%)
SEARCH | In Progress | Catalog 1 of 5 | MyPhotos | Update file types: 200 of 2000 (10%)
SEARCH | In Progress | Catalog 3 of 5 | Documents | Evaluated: 10254 of 100000 (10%) | Found: 2025
SEARCH | Paused      | Catalog 3 of 5 | Documents | Evaluated: 10254 of 100000 (10%) | Found: 2025
SEARCH | Stopped     | Catalog 3 of 5 | Documents | Evaluated: 2500 of 10000 (25%) | Found: 125
SEARCH | Completed   | Catalog 5 of 5 | Videos | Evaluated: 10000 of 10000 (100%) | Found: 250

//EXPLORE
EXPLORE | In Progress | Catalog 1 of 1 | MyPhotos | Loaded: 100 of 1000 (10%)
EXPLORE | In Progress | Catalog 1 of 1 | MyPhotos | Update file types: 200 of 2000 (10%)
(note: Pause or Stop is not available)

//UPDATE
UPDATE | In Progress | Catalog 1 of 5 | MyPhotos | Loaded: 100 of 1000 (10%)
UPDATE | In Progress | Catalog 1 of 1 | MyPhotos | Counted: 2000
UPDATE | In Progress | Catalog 1 of 5 | MyPhotos | Update file types: 200 of 2000 (10%)
UPDATE | In Progress | Catalog 1 of 5 | MyPhotos | Indexed: 200 of 2000 (10%) | /home/stephane/Document/test.odt
UPDATE | In Progress | Catalog 1 of 5 | MyPhotos | Extracted: 200 of 2000 (10%) | 1h 25m 10s | /home/stephane/Document/test.odt
(note: Pause is not available)
UPDATE | Cancelled | Catalog 1 of 1 | MyPhotos | Indexed: 2500 of 10000 (25%)
UPDATE | Completed | Catalog 1 of 1 | MyPhotos | Indexed: 1000 of 1000 (100%)
```

### Ideas for UI improvements
* DEV: normalize the parts size with spaces (Operation, State)
* DEV: color code the opeartion states

### Other Notes
* when triggering an update on a catalog from odler version (before 2.8), the step "File Types updated" is not the first step. It happens after the indexing step so that only remaining files are processed for file type update.

---


## **STATE MACHINE & ARCHITECTURE**

### Operation Phase State Machine

Catalog operations progress through explicit phases tracked by `CatalogManager`:
```
PHASE_IDLE
    ↓ (operation starts)
PHASE_COUNTING → (cancel) → emit catalogOperationCancelled() → PHASE_IDLE
    ↓ (counting completes)
PHASE_INDEXING → (cancel) → emit catalogOperationCancelled() → PHASE_IDLE
    ↓ (indexing completes, if needed)
PHASE_MIGRATING → (cancel) → emit catalogOperationCancelled() → PHASE_IDLE
    ↓ (all complete)
PHASE_COMPLETING
    ↓ (emit catalogOperationCompleted)
PHASE_IDLE
```

**Phase Detection Rules:**
- `PHASE_COUNTING`: When currentPath starts with `__COUNTING_STATE__|` or ends with `" files."`
- `PHASE_INDEXING`: When totalFiles > 0 and currentPath contains real file paths
- `PHASE_MIGRATING`: When currentPath starts with `__FILETYPE_MIGRATION__|`
- `PHASE_COMPLETING`: Set just before emitting catalogOperationCompleted

**Phase Persistence:**
- `m_currentPhase`: Current phase during operation
- `m_lastPhase`: Preserved when operation ends (for cancelled messages)

### Signal Flow & Responsibilities
```
CatalogJobStoppable
    ↓ (emits catalogProgress)
CatalogManager
    ├─ Updates: m_filesProcessed, m_totalFiles, m_currentPath
    ├─ Detects phase transitions → setOperationPhase()
    ├─ On completion: saves m_lastFilesProcessed, m_lastTotalFiles
    └─ Emits: catalogOperationCompleted / catalogOperationCancelled
        ↓
CatalogProgressManager
    ├─ Listens to progress signals → calls updateFromCatalogManager()
    ├─ Builds IN-PROGRESS messages (only when operation running)
    ├─ Listens to completion/cancelled signals
    └─ Builds FINAL messages using lastX values
```

**Message Building Responsibility:**
- **In-Progress**: `CatalogProgressManager::updateFromCatalogManager()` - uses current values
- **Cancelled**: `catalogOperationCancelled` lambda - uses `lastPhase()`, `lastFilesProcessed()`, `lastTotalFiles()`
- **Completed**: `catalogOperationCompleted` lambda - uses `lastFilesProcessed()`, `lastTotalFiles()`

**Critical Rule**: `updateFromCatalogManager()` does NOTHING when operation not running to prevent message replacement.

### State Preservation Pattern

The "last" value pattern ensures message data survives operation cleanup:
```cpp
// During operation:
setFilesProcessed(100);  // Updates both m_filesProcessed and m_lastFilesProcessed

// On completion (before cleanup):
m_lastFilesProcessed = results[1];  // Explicit save from job results
m_lastTotalFiles = results[1];

// After cleanup:
m_filesProcessed = 0;        // Cleared
// But m_lastFilesProcessed = 100  // Preserved for completion message

// Signal handlers use:
m_catalogManager->lastFilesProcessed()  // Always available
```

### Multi-Catalog Context (TBD)

For operations processing multiple catalogs (Virtual/Storage devices):
- **Catalog Index**: Managed by DeviceUpdateManager (knows "processing 2 of 5")
- **Catalog Name**: Managed by CatalogManager (knows "MyPhotos")
- **Message Building**: CatalogProgressManager combines both

*Architecture to be refined - see Issue #3 below.*
```

## 3. Multi-Catalog Index Issue - Recommendation

**Current Problem:**
```
UPDATE | In Progress | Catalog 1 of 1 | MyPhotos | ...  (processing catalog 1)
UPDATE | In Progress | Catalog 1 of 1 | Videos | ...    (processing catalog 2) ← WRONG
UPDATE | In Progress | Catalog 1 of 1 | Docs | ...      (processing catalog 3) ← WRONG

---
## **MESSAGE SOURCES TABLE**
This section documents all status bar messages in Katalog, tracking their implementation status and MessageBuilder adoption.

**Legend:**
- ✅ = Uses StatusBarMessageBuilder
- ❌ = Direct setText() / legacy format
- ⚠️ = Partial / needs improvement

### INITIALIZATION

| **Message Type** | **Example** | **Timeout** | **Uses Builder** | **Location** |
|------------------|-------------|-------------|------------------|--------------|
| Application ready | "Ready" | none | ❌ | `MainWindow` constructor timer |



**Notes:**
- Message shown after app initialization completes
- Currently uses direct setText
- Consider if this message is necessary (status bar could be empty initially)

* DEV: indeed, is this even displayed at any time? "Ready" is a default sate of conditions later (to be documented)




### CATALOG OPERATIONS

#### CatalogProgressManager (⚠️ Partially Migrated)

| **Message Type** | **Example** | **Timeout** | **Uses Builder** | **Status** |
|------------------|-------------|-------------|------------------|------------|
| **In Progress (CREATE)** | `CREATE \| In Progress \| Catalog 1 of 1 \| MyPhotos \| Indexed: 200 of 2000 (10%)` | none | ✅ | Working |
| **In Progress (UPDATE)** | `UPDATE \| In Progress \| Catalog 1 of 1 \| MyPhotos \| Indexed: 200 of 2000 (10%)` | none | ✅ | Working |
| Counting files | `CREATE \| In Progress \| Catalog 1 of 1 \| MyPhotos \| Counting files: 1250` | none | ✅ | Working |
| **Completed** | `CREATE \| Completed \| Catalog 1 of 1 \| MyPhotos \| Indexed: 2000 of 2000 (100%)` | 5000ms | ⚠️ | **ISSUE: Shows wrong operation/status** |
| **Stopped** | `UPDATE \| Stopped \| Catalog 1 of 1 \| MyPhotos` | 5000ms | ❌ | Legacy string "Operation cancelled" |
| Cancelled | "Operation cancelled" | 5000ms | ❌ | Direct setText |

**Critical Issues:**
1. **Completion branch (else)**: Needs to use MessageBuilder properly
2. **Operation type detection**: May return wrong type after job cleanup
3. **Cancelled handler**: Uses lambda with direct setText instead of MessageBuilder

**Implementation Method:** `CatalogProgressManager::updateFromCatalogManager()`

### SEARCH OPERATIONS

#### SearchProgressManager (✅ Fully Migrated)

| **Message Type** | **Example** | **Timeout** | **Uses Builder** | **Method** |
|------------------|-------------|-------------|------------------|------------|
| Search in progress | `SEARCH \| In Progress \| Catalog 1 of 3 \| Documents \| Evaluated: 10254 of 100000 (10%) \| Found: 2025` | none | ✅ | `updateFromSearchManager()` |
| Search completed | `SEARCH \| Completed \| Found: 1250` | 5000ms | ✅ | `updateFromSearchManager()` |
| Search paused | Current message + " \| PAUSED" | none | ✅ | `updateFromSearchManager()` |

**Implementation:** SearchProgressManager fully migrated to StatusBarMessageBuilder



#### MainWindow Search (✅ Fully Migrated)

| **Message Type** | **Example** | **Timeout** | **Uses Builder** | **Method** |
|------------------|-------------|-------------|------------------|------------|
| Catalog loading | `SEARCH \| In Progress \| Catalog 1 of 3 \| MyPhotos \| Loading: 100 of 1000 (10%)` | none | ✅ | `updateSearchProgress()` |
| File type conversion | `SEARCH \| In Progress \| Catalog 1 of 3 \| MyPhotos \| Update file types: 200 of 2000 (10%)` | none | ✅ | `updateSearchProgress()` |
| Search stopped | `SEARCH \| Stopped \| Files found: 125` | 5000ms | ✅ | `updateSearchProgress()` |
| Search ready | "Ready" | none | ❌ | `updateStatusBarFromSearchManager()` |

**Notes:**
- updateSearchProgress() fully uses MessageBuilder
- "Ready" state still uses direct setText



### DEVICE UPDATE OPERATIONS

#### MainWindow Device Updates (❌ Not Migrated)

| **Message Type** | **Example** | **Timeout** | **Uses Builder** | **Method** |
|------------------|-------------|-------------|------------------|------------|
| Update progress | "Scanning files: 500/1000 (50%) - MyDevice" | none | ❌ | `onDeviceUpdateProgress()` |
| Update error | "Operation cancelled" | 5000ms | ❌ | `onDeviceUpdateError()` |
| Update cancelled | "Operation cancelled" | 3000ms | ❌ | `onDeviceUpdateCancelled()` |
| UI state clear | (clears message) | - | N/A | `setCatalogUpdateUIState(false)` |

**Notes:**
- All methods use direct setText with simple string concatenation
- Should be migrated to use MessageBuilder for consistency
- onDeviceUpdateProgress() only called when CatalogProgressManager is NOT handling the operation

**Target Format:**
```
UPDATE | In Progress | MyDevice | Scanned: 500 of 1000 (50%)
```

---

### EXPLORE TAB

#### Explore Operations (⚠️ Mixed)

| **Message Type** | **Example** | **Timeout** | **Uses Builder** | **Method** |
|------------------|-------------|-------------|------------------|------------|
| File type conversion | `EXPLORE \| In Progress \| MyPhotos \| Update file types: 200 of 2000 (10%)` | none | ✅ | Uses shared code path |
| Migration cancelled | "Migration cancelled" | 2000ms | ❌ | `on_Explore_pushButton_OpenCatalog_clicked()` |
| Catalog ready | "Catalog ready" | 2000ms | ❌ | `on_Explore_pushButton_OpenCatalog_clicked()` |

**Notes:**
- File type conversion uses same code as Search/Catalog operations
- Simple status messages still use direct setText

---

### CREATE TAB

#### Create Operations (⚠️ Critical Issue)

| **Message Type** | **Example** | **Timeout** | **Uses Builder** | **Status** |
|------------------|-------------|-------------|------------------|------------|
| **In Progress** | `CREATE \| In Progress \| Catalog 1 of 1 \| MyPhotos \| Indexed: 200 of 2000 (10%)` | none | ✅ | Working via CatalogProgressManager |
| **Completed** | `CREATE \| Completed \| Catalog 1 of 1 \| MyPhotos \| Indexed: 2000 of 2000 (100%)` | 5000ms | ⚠️ | **GETS ERASED** |
| Stopped/Error | "Operation cancelled" | 5000ms | ❌ | Direct setText |

**Critical Issue:**
- `setCreateCatalogUIState(false)` calls `statusBarLabel->clear()` 
- This **ERASES** the completion message immediately after CatalogProgressManager displays it
- **Fix Required:** Remove the clear() call, let CatalogProgressManager handle timeout

**Location:** `src/mainwindow_tab_create.cpp` line ~2157

---

### MIGRATION PRIORITIES

#### High Priority (User-Visible Issues)
1. 🔴 **CatalogProgressManager completion** - Shows wrong operation/status
2. 🔴 **CREATE statusBarLabel->clear()** - Erases completion message
3. 🔴 **Cancelled operations** - Legacy "Operation cancelled" string

#### Medium Priority (Consistency)
4. 🟡 **onDeviceUpdateProgress()** - Should use MessageBuilder
5. 🟡 **onDeviceUpdateError()** - Should use MessageBuilder
6. 🟡 **onDeviceUpdateCancelled()** - Should use MessageBuilder

#### Low Priority (Simple Messages)
7. 🟢 **"Ready" states** - Consider if needed at all
8. 🟢 **Explore tab simple messages** - "Migration cancelled", "Catalog ready"

---

### STANDARDIZATION CHECKLIST

#### ✅ Completed
- [x] SearchProgressManager fully migrated
- [x] MainWindow::updateSearchProgress() migrated
- [x] CatalogProgressManager in-progress messages
- [x] Rich text support (QLabel with HTML)
- [x] StatusBarMessageBuilder implementation
- [x] Translation optimization (normal case → uppercase)

#### ⚠️ In Progress
- [ ] CatalogProgressManager completion branch
- [ ] CREATE operation completion display
- [ ] Cancelled operation messages

#### ❌ Not Started
- [ ] onDeviceUpdateProgress() migration
- [ ] onDeviceUpdateError() migration
- [ ] onDeviceUpdateCancelled() migration
- [ ] Explore tab simple messages
- [ ] "Ready" state standardization

---

### MESSAGE FORMAT VERIFICATION

All messages using StatusBarMessageBuilder should follow this format:

```
OPERATION | Status | Device Context | Process: X of Y (Z%) | Result: N | Current Item
```

**Examples:**
```
✅ SEARCH | In Progress | Catalog 3 of 5 | Documents | Evaluated: 10254 of 100000 (10%) | Found: 2025
✅ UPDATE | In Progress | Catalog 1 of 1 | MyPhotos | Indexed: 200 of 2000 (10%) | /home/user/file.txt
✅ CREATE | Completed | Catalog 1 of 1 | MyPhotos | Indexed: 2000 of 2000 (100%)
❌ "Scanning files: 500/1000 (50%) - MyDevice"  ← Legacy format
❌ "Operation cancelled"  ← Legacy format
```
---

### Implementation

```cpp
class StatusBarMessageBuilder {
//see source code
};

// Usage:
QString message = StatusBarMessageBuilder()
    .setOperation(tr("Search"))  // Auto-converted to "SEARCH" uppercase
    .setStatus(tr("In Progress"))
    .setDeviceContext(1, 5, "MyPhotos")
    .setProcess(tr("Update file types"), 200, 2000)
    .build();
// Result: "SEARCH | In Progress | Catalog 1 of 5 | MyPhotos | Update file types: 200 of 2000 (10%)"

// Search file matching example:
QString message = StatusBarMessageBuilder()
    .setOperation(tr("Search"))
    .setStatus(tr("In Progress"))
    .setDeviceContext(3, 5, "Documents")
    .setProcess(tr("Evaluated"), 10254, 100000)
    .setResult(tr("Found"), 2025)
    .build();
// Result: "SEARCH | In Progress | Catalog 3 of 5 | Documents | Evaluated: 10254 of 100000 (10%) | Found: 2025"

// CREATE completion example:
QString message = StatusBarMessageBuilder()
    .setOperation(tr("Create"))
    .setStatus(tr("Completed"))
    .setDeviceContext(1, 1, "MyPhotos")
    .setProcess(tr("Indexed"), 2000, 2000)
    .build();
// Result: "CREATE | Completed | Catalog 1 of 1 | MyPhotos | Indexed: 2000 of 2000 (100%)"
```

**Terminology Notes:**
- **"In Progress"** replaces "Running" for clearer translations across 30 languages
- **"Evaluated"** used for search file matching (comparing files against criteria)
- **"Indexed"** used for catalog create/update operations (files added to or scanned in catalog database)
- **Device Context format:** "Catalog X of Y | CatalogName" (pipe separator, catalog name in theme blue color)
- **Device Context always shown:** Even for single catalogs (1 of 1) to maintain consistency

#### Other Requirements

1. **Operation names**: Translated as normal case (e.g., tr("Search"), tr("Update"), tr("Create")) and auto-converted to uppercase (e.g., "SEARCH", "UPDATE", "CREATE") by the builder to minimize translation strings

2. **Status names**: Translated as normal case (e.g., tr("In Progress"), tr("Completed"), tr("Paused"))

3. **Separators**:  " | " 

4. **Percent formatting**: "10%" or "10.0%" (configurable, currently set to integer: "10%")

5. **Current item path**: Not truncated if too long (alternative could be ".../Documents/test.odt")

6. **Rich text support**: Messages support HTML formatting (bold, colors, italic) via QLabel widget in status bar

7. **Batched updates**: Current file path (Part 10) updates every ~50-100 files processed to balance visual feedback with UI performance
                   
### Implementation Status

#### Completed Features

1. **Rich Text Support**
   - Added `QLabel* statusBarLabel` to MainWindow for HTML-formatted messages
   - Messages now support bold, colors, and italic formatting
   - Operation names displayed in bold by default
   - Catalog names displayed in theme blue (#39b2e5)

2. **Status Field**
   - Added dedicated Status field to StatusBarMessageBuilder
   - Allows separate display of operation state (In Progress, Completed, Paused, etc.)
   - Message structure: Operation | Status | Device Context | Process | Result | Current Item

3. **Translation Optimization**
   - Added `operationUppercase` option to FormatOptions (default: true)
   - Operations use normal case translations (tr("Search")) auto-converted to uppercase ("SEARCH")
   - Reduces translation strings: use tr("Search") instead of separate tr("SEARCH")

4. **Progress Manager Updates**
   - SearchProgressManager updated to use QLabel for rich text
   - CatalogProgressManager updated to use QLabel for rich text
   - Both managers now support HTML-formatted messages
   - Fixed directory search progress display (shows "Evaluated" count instead of "Searching in directory")

5. **MainWindow Integration**
   - Added `updateStatusBarMessage(QString, int)` helper method
   - All MainWindow status messages converted to use statusBarLabel
   - Consistent timeout handling via statusBarTimer

6. **Current File Path Display**
   - Added `currentFilePath` member to `SearchJobStoppable`
   - Directory searches now display current file path being processed
   - File path updates use same batching as catalog searches (~50-100 files per update)
   - Prevents UI spam while providing progress feedback

#### Current Format Options

```cpp
struct FormatOptions {
    bool operationBold = true;           // Operation text in bold
    bool operationUppercase = true;      // Auto-convert to uppercase
    QString operationColor = "";         // Operation color (empty = default)
    QString catalogNameColor = "#39b2e5"; // Catalog name color (Katalog theme blue)
    bool resultsBold = true;             // Result numbers in bold
    QString processColor = "";           // Process text color (empty = default)
    bool currentItemItalic = true;       // Current item in italic
};
```

#### Known Issues to Address

1. **CatalogProgressManager Completion Messages**
   - Issue: Completion branch (when operation not running) needs to use StatusBarMessageBuilder
   - Current: Uses legacy string-based messages
   - Target: Use MessageBuilder with proper operation type detection and "Completed" status

2. **CREATE Operation Status Bar Clear**
   - Issue: `setCreateCatalogUIState(false)` calls `statusBarLabel->clear()` which erases completion message
   - Target: Remove clear() call and let CatalogProgressManager handle message with timeout

3. **onDeviceUpdateProgress() Messages**
   - Status: Not yet migrated to StatusBarMessageBuilder
   - Target: Convert to use MessageBuilder format

#### Next Steps

- Fix CatalogProgressManager completion messages to use StatusBarMessageBuilder
- Remove premature statusBarLabel->clear() in CREATE operation completion
- Continue migrating remaining messages to use StatusBarMessageBuilder
- Uncomment additional formatting options as needed (colors, italic)
- Update remaining message sources in MESSAGE SOURCES TABLE
                   
### Changes from Previous Version

#### Added
- **"Indexed"** process title for CREATE and UPDATE catalog operations
- HTML color formatting note for catalog names (#39b2e5 blue)
- Device Context shown even for single catalogs (1 of 1)
- CREATE completion examples showing proper format
- Known Issues section documenting current problems

#### Changed
- **Process Title table**: Added "Indexed" to Update and Create operations
- **Device Context description**: Clarified catalog name uses HTML color and shown for all operations
- **Examples**: Updated to use "Indexed" instead of "Processed" for catalog operations
- **Message Examples section**: Added proper CREATE completion format
- **Terminology Notes**: Added "Indexed" definition and Device Context clarifications

#### Clarified
- Device Context format includes HTML-colored catalog name
- Both CREATE and UPDATE operations use "Indexed" for file processing
- Completion messages should use StatusBarMessageBuilder consistently
                   
### Other ideas
* ~~rich text for the messages~~ ✅ **IMPLEMENTED** - Using QLabel for HTML formatting
* continue to unify the use of the class for all messages
* Add color coding for different operation states (running=blue, completed=green, error=red, paused=orange)
* Add progress bar widget alongside text messages for long-running operations

### **QUESTIONS FOR CLARIFICATION:**

Before proceeding with implementation, I need to understand your requirements better:

1. **Scope of Unification:**
   - Should we unify ALL status bar messages through a single system, including the direct MainWindow calls?
   - Or maintain the existing manager pattern but ensure consistency?

2. **Message Priority:**
   - When multiple operations could report simultaneously, what priority should they have?
   - Should newer messages always override, or should some operations "lock" the status bar?

3. **Timeout Behavior:**
   - Should all messages have configurable timeouts?
   - Current pattern: running operations have no timeout, completions have 5s timeout. Keep this?

4. **Progress Manager Pattern:**
   - The existing `SearchProgressManager` and `CatalogProgressManager` work well. Should we:
     - Create a base `StatusBarProgressManager` class they inherit from?
     - Create a unified `StatusBarManager` that all components use?
     - Keep separate managers but standardize their interface?

5. **DeviceUpdateManager Integration:**
   - Should `DeviceUpdateManager` have its own `DeviceProgressManager`?
   - Or route its messages through `CatalogProgressManager` (as it currently partially does)?






