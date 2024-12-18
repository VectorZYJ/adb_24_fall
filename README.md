# Replicated Concurrency Control and Recovery Database

Yujia Zhu, yz10317, yz10317@nyu.edu

## Design document

For this project, we firstly design two entities: Operation and Transaction, and then two main components for RepCRec: Transaction Manager and Data Site.

### Operation

Each operation has the following attributes: time, type(read, write or some else), transaction_id(belong to which transaction), site_id(operate on which site), value(for write operation).

### Transaction

Each transaction has the following attributes: id, start_time, end_time, status(waiting or commited), operation_list, locally writes (hasn't committed), write_set, read_set.


### Transaction Manager

- The Transaction Manager is used to translate read and write requests on variables to read and write requests on copies using the available copy algorithm.

- Send read and write requests to a Data Site; Monitor the availability of each site and reroutes transactions if a site fails

- Main functions
  1. `begin(t_id, time)`: starts a new transaction at current time.
  2. `read(t_id, operation)`: takes a read type operation as input, return the committed value as output; If a site is down, it retries at another site. Transaction waits if no sites are available.
  3. `write(t_id, operation)`: takes a write type operation as input, write all available copies of a variable with the value.
  4. `dump()`: prints out the committed values of all copies of all variables at all sites.
  5. `validate(t_id, time)`: report whether a transaction can commit or abort in the format
  6. `fail(site_id)`: cause a site to fail
  7. `recover(site_id)`: cause a site to recover
  8. `commit(t_id, time)`: commit a transaction, ensure serializable snapshot isolation using the First Committer Wins rule and by aborting transactions in the presence of consecutive RW edges in a cycle, also check available rule
  9. `abort(t_id, time)`: cause a transaction to abort

### Site

- Stores and manages copies of data items. Data Manager in each site controls access to its local copy of data variables.

- Receives read and write requests from the Transaction Manager. Reports failure/recovery to Transaction Manager. Handles commit and abort requests based on Transaction Manager instructions.

- Main functions
  1. `get_record(x_id, time)`: read the last committed variable prior to certain time at this site
  2. `update_record(x_id, value, time)`: write a value to variable at this site
  3. `fail()`: cause this site to fail
  4. `recover()`: recover this site

## Usage

### Project build

**Environment setting**: Cmake 3.31.2, GCC/G++ 14.0.3, GNU Make 3.81

Just run `./build.sh` to build the project

**Detailed step are listed below**

Create a build directory:
```shell
mkdir build
cd build
```

Generate build files:
```shell
cmake ..
```

Build the project:
```shell
make
```

Move executable file to home directory and return back:
```shell
mv RepCRec ..
cd ..
```

### Project run

Just use `./run.sh` to run the RepCRec project

Run the project:
```shell
## Run without filename, using interactive mode
./RepCRec

## Or run with filename, as input file
./RepCRec <filename>
```

Test files are located in `input` directory, with an extra file `expected_output` to explain outcome for each test data.

### Input format

RepCRec allows these instructions below:

- `begin(T1)`: Transaction `1` begins
- `R(T1, x1)`: Transaction `1` wishes to read `x1`
- `W(T1, x1, v)`: Transaction `1` wishes to write value `v` to all available copies of `x1`
- `dump()`: Prints out committed values for all copies of all variables at all sites
- `end(T1)`: Transaction `1` ends, report whether it can commit or abort
- `fail(1)`: Site `1` fails
- `recover(1)`: Site `1` recovers
- `End`: only for interactive mode, end of all instructions