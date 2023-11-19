# Mu-Controls-Backend
This is the Go backend server for processing data from the Teensy and serve data to the Flutter GUI


### Install / Setup

1. Install Go in your local environment. I use homebrew to manage my go installs on MacOS and Linux.

2. (Optional) Install GoDotEnv to use .env variables to avoid committing secrets to the repository. PLEASE DO NOT COMMIT any passwords, api keys, or other secrets. Period. Ever. If you do, we will have to wipe this repository and its history clean to remove the record.
I used the following command from the github page to install on my Mac: `go install github.com/joho/godotenv/cmd/godotenv@latest`

3. Set your environment variables (or get them from one of the devs working on this). This is the current list of environment variables:
  - (this is still a work in progress)
  - `GOPATH`
  - `POSTGRES_PASSWORD`

4. Install Docker on your local system or whatever system will be running the server

5. Run `docker compose up` to start the database instance. This will also initialize it on the first run if you've set your environment variables appropriately.
