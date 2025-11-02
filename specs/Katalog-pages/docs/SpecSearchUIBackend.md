#Search UI vs backend architecture

**Excellent architectural question!** You're absolutely right to dig into this. Let me diagnose the current architecture:

---

## **Current Architecture Analysis**

### **Backend (Good - Reusable) ✅**
```
SearchJobStoppable
├─ Pure search logic
├─ Emits: searchProgress(int)
└─ No UI dependencies

SearchManager  
├─ Manages search lifecycle
├─ Emits: statusChanged, progressChanged, searchRunning
└─ No UI dependencies

StatusBarMessageBuilder
├─ Pure message construction
├─ Returns: QString
└─ No UI dependencies
```

### **Middle Layer (PROBLEMATIC) ⚠️**
```
SearchProgressManager
├─ Receives signals from SearchManager
├─ Builds messages using StatusBarMessageBuilder
├─ BUT: Constructor takes QStatusBar*, QTimer*, QLabel*
├─ Directly manipulates: m_statusBarLabel->setText()
└─ VERDICT: Tightly coupled to Qt Widgets UI!

MainWindow::updateSearchProgress()
├─ Receives searchProgress(int) signal
├─ Builds messages using StatusBarMessageBuilder
├─ Directly updates: statusBarLabel->setText()
└─ VERDICT: Mixing message logic (backend) with UI update
```

---

## **The Problem**

**SearchProgressManager pretends to be backend but it's actually UI:**
- Takes UI widgets in constructor
- Directly manipulates QLabel/QStatusBar
- Can't be reused in QtQuick

**MainWindow::updateSearchProgress() mixes concerns:**
- Message construction logic (should be backend)
- UI widget manipulation (should be UI)

---

## **Clean Architecture Should Be:**

```
┌─────────────────────────────────────┐
│ BACKEND (Reusable)                  │
├─────────────────────────────────────┤
│ SearchJobStoppable                  │
│   └─ emit searchProgress(int)       │
│                                     │
│ SearchProgressCoordinator           │ <-- NEW CLASS
│   ├─ Receives searchProgress(int)  │
│   ├─ Receives SearchManager signals │
│   ├─ Builds message using Builder   │
│   └─ emit messageReady(QString)     │ <-- Pure signal
└─────────────────────────────────────┘
           │
           │ Qt Signal (QString)
           ▼
┌─────────────────────────────────────┐
│ UI LAYER (Qt Widgets)               │
├─────────────────────────────────────┤
│ MainWindow                          │
│   └─ statusBarLabel->setText(msg)  │
└─────────────────────────────────────┘
           OR
┌─────────────────────────────────────┐
│ UI LAYER (QtQuick)                  │
├─────────────────────────────────────┤
│ QML Component                       │
│   └─ text: progressMessage          │
└─────────────────────────────────────┘
```

---

## **Diagnosis Summary**

| **Component** | **Current State** | **Should Be** |
|---------------|-------------------|---------------|
| SearchJobStoppable | ✅ Pure backend | ✅ Keep as-is |
| SearchManager | ✅ Pure backend | ✅ Keep as-is |
| StatusBarMessageBuilder | ✅ Pure backend | ✅ Keep as-is |
| **SearchProgressManager** | ⚠️ **UI-coupled** | 🔧 **Remove UI deps, emit signals** |
| **MainWindow::updateSearchProgress()** | ⚠️ **Mixed logic** | 🔧 **Only UI updates, no message logic** |

---

## **Recommendation for QtQuick Readiness**

**Refactor SearchProgressManager to be truly backend:**

```cpp
class SearchProgressCoordinator : public QObject {  // Rename for clarity
    Q_OBJECT
public:
    // NO UI dependencies in constructor
    SearchProgressCoordinator(QObject *parent = nullptr);
    
    void connectToSearchManager(SearchManager *manager);
    void setCurrentSearch(Search *search);

signals:
    void progressMessageReady(const QString &message);  // Pure signal
    void messageTimeout(int ms);                        // Tell UI when to hide

private slots:
    void updateFromSearchManager();
    void updateFromSearchProgress(int filesProcessed);
    
private:
    SearchManager *m_searchManager = nullptr;
    Search *m_currentSearch = nullptr;
    // NO QLabel, QStatusBar, QTimer!
};
```

**Then MainWindow becomes pure UI:**
```cpp
void MainWindow::setupSearchProgressDisplay() {
    connect(searchProgressCoordinator, &SearchProgressCoordinator::progressMessageReady,
            this, [this](const QString &msg) {
                statusBarLabel->setText(msg);  // Pure UI
                statusBar()->show();
            });
    
    connect(searchProgressCoordinator, &SearchProgressCoordinator::messageTimeout,
            statusBarTimer, &QTimer::start);  // Pure UI
}
```

---

**For QtQuick migration, you'd just replace the MainWindow connections with QML bindings!**

**Does this diagnosis make sense? Should we refactor SearchProgressManager to remove UI dependencies?**
