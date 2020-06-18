CREATE TABLE IF NOT EXISTS "jerk" (
	"id" SERIAL PRIMARY KEY,
	"event_starttime" BIGINT NULL,
	"starttime" BIGINT NOT NULL,
	"endtime" BIGINT NULL,
	"phase" INTEGER NOT NULL,
	"num_points" INTEGER NOT NULL,
	"min" REAL NOT NULL,
	"max" REAL NOT NULL,
	"avg" REAL NOT NULL,
	CONSTRAINT "FKEvent" FOREIGN KEY ("event_starttime") REFERENCES "events" ("starttime") ON UPDATE CASCADE ON DELETE CASCADE
);