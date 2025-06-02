#pragma once

#include "jade/db/MigrationInstance.hpp"
namespace jade {

struct MInitial : public MigrationInstance {
    
    void upgrade(pqxx::work& tx) override {
        tx.exec(R"(
        -- Object tables {{{
        CREATE TABLE Jade.Users (
            UserID      SERIAL          PRIMARY KEY,
            Username    TEXT UNIQUE     NOT NULL,
            Password    TEXT            NOT NULL,
            Salt        TEXT            NOT NULL,
            Version     INTEGER         NOT NULL,
            IsAdmin     BOOLEAN         NOT NULL
        );
        CREATE TABLE Jade.Libraries (
            LibraryID   SERIAL          PRIMARY KEY,
            Location    TEXT            NOT NULL,
            AgeRating   INTEGER
        );
        CREATE TABLE Jade.Tags (
            TagID       SERIAL          PRIMARY KEY,
            TagName     TEXT            NOT NULL        UNIQUE,
            TagType     INTEGER         NOT NULL        DEFAULT 0
        );
        CREATE TABLE Jade.Authors (
            AuthorID SERIAL PRIMARY KEY,
            AuthorName TEXT NOT NULL UNIQUE
        );
        CREATE TABLE Jade.Books (
            BookID      SERIAL          PRIMARY KEY,
            FileName    TEXT            NOT NULL,
            LibraryID   INTEGER         REFERENCES Jade.Libraries(LibraryID) ON DELETE CASCADE,
            Sequel      INTEGER         REFERENCES Jade.Books(BookID) ON DELETE SET NULL,
            Prequel     INTEGER         REFERENCES Jade.Books(BookID) ON DELETE SET NULL,
            Title       TEXT            NOT NULL,
            ISBN        TEXT,
            Description TEXT,
            AgeRating   INTEGER
        );
        -- }}}
        -- Relational tables {{{
        CREATE TABLE Jade.BookTags (
            BookID INTEGER NOT NULL REFERENCES Jade.Books(BookID) ON DELETE CASCADE,
            TagID  INTEGER NOT NULL REFERENCES Jade.Tags(TagID) ON DELETE CASCADE,
            PRIMARY KEY (BookID, TagID)
        );
        CREATE TABLE Jade.BookAuthors (
            BookID INTEGER NOT NULL REFERENCES Jade.Books(BookID) ON DELETE CASCADE,
            AuthorID  INTEGER NOT NULL REFERENCES Jade.Authors(AuthorID) ON DELETE CASCADE,
            PRIMARY KEY (BookID, AuthorID)
        );
        -- }}}
        )");
    }

    void downgrade(pqxx::work& tx) override {
        // Downgrading the initial database version is equivalent to just wiping it
        tx.exec("DROP SCHEMA Jade CASCADE;");
    }

    std::string name() override { return "Initial database setup"; }
};

}
