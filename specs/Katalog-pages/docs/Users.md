Hi Manu, 
Thanks for reaching out!

Here some roadmap info to answer:

## db server
### db server (localhost or remote) with current model
Yes, I have done some pre-development & testing for using "remote" databases.
https://github.com/StephaneCouturier/Katalog/issues/402
Getting the SQLite mode to work was the first step, this could be good candidate for v2.9.
I only tested this using a db service like Supabase. It runs Postgres db, so there might still be some work to ensure all sql queries in Katalog are compatible or manage the specific cases.
Credentials would be stored in local Katalog_settings.ini as a start, but this may not be ideal for security.
Performance is definitly a challenge though.
I am mostly curious to test the performance of a db server on my local machine (MariaDB/MySQL), it should be faster than the SQLite file, so I will test next as well.

### db with serverside RESTful API model
Working in API mode, so getting the server to hold and execute the SQL, would be a major change in the architecture, or will require to manage both modes in parallel. 
Big task.
But I'd be curious to starting experimenting it at least.

## clients
### webservice 
Never started.

### Mobile App
I have successfully created a working basic version of Katalog with QtQuick/Kirigami.
It can load a SQLite db file, enable a device selection, and run a Search returing results.
It could be Katalog 3.x with tentaive mobile release, but this is also a big task.

---

got a question from user:
would you consider the idea, to possibly add a serverside API, so it would add some stability to develop a web-view against it?
maybe you would even consider to wrap the sql requests into some http transport in the first place. that would help to open the database to internet access.

what does "even consider to wrap the sql requests into some http transport in the first place" mean and how would it apply to KAtalog?


I'll search the project knowledge to understand KAtalog's current architecture and then explain this concept.Based on my review of KAtalog's architecture, let me explain what "wrap the sql requests into some http transport in the first place" means and its implications:

## Current Architecture

KAtalog currently has a **direct database access model**:
```
Desktop Application → Qt SQL (QSqlDatabase/QSqlQuery) → SQLite Database File
```

The application directly executes SQL queries like:
- `SELECT * FROM device WHERE device_id = ?`
- `INSERT INTO catalog VALUES (...)`
- `UPDATE file SET file_size = ? WHERE file_id = ?`

## What "Wrapping SQL in HTTP Transport" Means

This would transform the architecture to a **client-server model**:

```
Client App → HTTP Requests → Web API Server → Database Layer → SQLite
```

Instead of:
```cpp
QSqlQuery query(QSqlDatabase::database(m_connectionName));
query.prepare("SELECT * FROM device WHERE device_id = ?");
query.bindValue(":device_id", deviceId);
query.exec();
```

You would have:
```cpp
// Client sends HTTP request
HTTPClient::get("/api/devices/" + deviceId)
// Server receives request, executes SQL internally, returns JSON
```

## How This Would Apply to KAtalog

**The transformation would involve:**

1. **Backend API Server** (new component):
   - RESTful API endpoints like:
     - `GET /api/devices` - list all devices
     - `GET /api/catalogs/{id}/files` - get files in a catalog
     - `POST /api/catalogs` - create new catalog
     - `PUT /api/devices/{id}` - update device
   - Server handles all SQL queries internally
   - Returns JSON responses to clients

2. **Refactor Database Layer** (`database.cpp`, `device.cpp`, `catalog.cpp`):
   - Current direct SQL code would move to server-side
   - Desktop app would use HTTP client instead of QSqlDatabase

3. **Multiple Client Types**:
   - Desktop Qt app (current)
   - Web interface (new)
   - Mobile apps (future)
   - All using the same HTTP API

## Trade-offs

**Benefits:**
- Web interface possible
- Multiple clients can access same database safely
- Centralized business logic
- Better security (database not directly exposed)

**Challenges:**
- **Major architectural rewrite** (thousands of lines affected)
- Performance overhead (network latency)
- Complexity increase
- Offline usage becomes complicated
- The app currently works with local database files - this changes that model fundamentally

## Question for You

Before suggesting implementation approaches, I need to understand your use case better:

1. **What's your primary goal?**
   - Enable web browser access to catalogs?
   - Share catalogs across network?
   - Both?

2. **Usage pattern:**
   - Would users still have local database files, OR
   - Would there be a central server hosting all catalogs?

3. **Existing functionality:**
   - The current app works with local `.db` files that users create/open
   - Should this workflow be preserved alongside the API, or replaced?

Once I understand your goals, I can suggest the best approach - it might not require wrapping ALL SQL in HTTP. There could be hybrid approaches that preserve the current local file model while adding remote access capabilities.
