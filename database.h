
#ifndef _DATABASE_H
#define  _DATABASE_H

#include "util.h"
#include <string.h>

void toBeginning(int fd){
    lseek(fd, 0, SEEK_SET);
}

int lock(int fd, short lock_type, int size, int record){
    struct flock flk = {
             .l_type = lock_type,
             .l_whence = SEEK_SET,
             .l_start = size * record,
             .l_len = size,
             .l_pid = getpid()
           };
    return fcntl(fd, F_SETLKW, &flk);
}

int createAccount(struct Account *account){

    if (account->id!=-1 )return -1;

    int metadata_fd = open(METADATA_DB_PATH, O_RDWR);

    struct Metadata metadata;

    lock(metadata_fd, F_WRLCK, 0, 0);
        sleep(DELAY);
        read(metadata_fd, &metadata, sizeof(metadata));
        account->id = metadata.account_id;
        metadata.account_id++;
        toBeginning(metadata_fd);
        write(metadata_fd, &metadata, sizeof(metadata));
    lock(metadata_fd, F_UNLCK, 0, 0);


    int account_fd = open(ACCOUNT_DB_PATH, O_APPEND | O_WRONLY);
    write(account_fd, account, sizeof(*account));

    close(metadata_fd);
    close(account_fd);
    return 0;

}

int createTransaction(struct Transaction *transaction){

    if (transaction->id!=-1 )return -1;

    int metadata_fd = open(METADATA_DB_PATH, O_RDWR);

    struct Metadata metadata;

    lock(metadata_fd, F_WRLCK, 0, 0);
        sleep(DELAY);
        read(metadata_fd, &metadata, sizeof(metadata));
        transaction->id = metadata.transaction_id;
        metadata.transaction_id++;
        toBeginning(metadata_fd);
        write(metadata_fd, &metadata, sizeof(metadata));
    lock(metadata_fd, F_UNLCK, 0, 0);


    int transaction_fd = open(TRANSACTION_DB_PATH, O_APPEND | O_WRONLY);
    write(transaction_fd, transaction, sizeof(*transaction));

    close(metadata_fd);
    close(transaction_fd);
    return 0;

}

int getTransactions(int account_id, struct Transaction transactions[], int maxcount){

    int fd = open(TRANSACTION_DB_PATH, O_RDONLY);
    if(fd == -1){
        perror("open() ");
        return 0;
    }

    int len;
    int count = 0;
    lock(fd, F_RDLCK, 0, 0);
    sleep(DELAY);
    struct Transaction transaction;
    while(len = read(fd, &transaction, sizeof(transaction))){
        if (transaction.account_id == account_id){
            transactions[count++] = transaction;
        }
        if(count == maxcount) break;
    }
    lock(fd, F_UNLCK, 0, 0);
    close(fd);
    return count;

}

int getUsers(struct User users[], int maxcount){

    int fd = open(USER_DB_PATH, O_RDONLY);
    if(fd == -1){
        perror("open() ");
        return 0;
    }

    int len;
    int count = 0;
    lock(fd, F_RDLCK, 0, 0);
    sleep(DELAY);
    struct User user;
    while(len = read(fd, &user, sizeof(user))){
        if (~user.id){
            users[count++] = user;
        }
        if(count == maxcount) break;
    }
    lock(fd, F_UNLCK, 0, 0);
    close(fd);
    return count;

}

int getAccounts(struct Account accounts[], int maxcount){

    int fd = open(ACCOUNT_DB_PATH, O_RDONLY);
    if(fd == -1){
        perror("open() ");
        return 0;
    }

    int len;
    int count = 0;
    lock(fd, F_RDLCK, 0, 0);
    sleep(DELAY);
    struct Account account;
    while(len = read(fd, &account, sizeof(account))){
        if (~account.id){
            accounts[count++] = account;
        }
        if(count == maxcount) break;
    }
    lock(fd, F_UNLCK, 0, 0);
    close(fd);
    return count;

}

int createUser(struct User *user){


    if (user->id!=-1) return -1;


    int metadata_fd = open(METADATA_DB_PATH, O_RDWR);
    if(metadata_fd == -1){
        perror("open");
    }

    struct Metadata metadata;

    lock(metadata_fd, F_WRLCK, 0, 0);
        read(metadata_fd, &metadata, sizeof(metadata));
        user->id = metadata.user_id;
        metadata.user_id++;
        toBeginning(metadata_fd);
        write(metadata_fd, &metadata, sizeof(metadata));
        sleep(DELAY);
    lock(metadata_fd, F_UNLCK, 0, 0);

    if (user->account_id == -1){
        struct Account account = {
            .balance = 1000,
            .id = -1
        };
        createAccount(&account);
        user->account_id = account.id;
    }


    int user_fd = open(USER_DB_PATH, O_APPEND | O_WRONLY);
    if(user_fd == -1){
        perror("open");
    }

    write(user_fd, user, sizeof(*user));

    close(metadata_fd);
    close(user_fd);
    return 0;

}

int getUser(char email[], struct User *user){

    int fd = open(USER_DB_PATH, O_RDONLY);
    if(fd == -1){
        perror("open() ");
    }

    int len;

    lock(fd, F_RDLCK, 0, 0);
    sleep(DELAY);
    while(len = read(fd, user, sizeof(*user))){
        if (strcmp(user->email, email) == 0 && ~user->id){
            close(fd);
            return 0;
        }

    }
    lock(fd, F_UNLCK, 0, 0);

    close(fd);
    return -1;
}

int getUserById(int user_id, struct User *user){

    int fd = open(USER_DB_PATH, O_RDONLY);
    if(fd == -1){
        perror("open() ");
    }

    int len;

    lock(fd, F_RDLCK, 0, 0);
    sleep(DELAY);
    while(len = read(fd, user, sizeof(*user))){
        if (user->id == user_id && ~user->id){
            close(fd);
            return 0;
        }

    }
    lock(fd, F_UNLCK, 0, 0);

    close(fd);
    return -1;
}

int getAccount(int account_id, struct Account *account){

    int fd = open(ACCOUNT_DB_PATH, O_RDONLY);
    if(fd == -1){
        perror("open() ");
    }

    int len;

    lock(fd, F_RDLCK, 0, 0);
    while(len = read(fd, account, sizeof(*account))){
        if (account->id == account_id){
            close(fd);
            return 0;
        }

    }
    lock(fd, F_UNLCK, 0, 0);

    close(fd);
    return -1;
}

int changeAccountBalance(int account_id, float amount){

    struct Account account;

    int fd = open(ACCOUNT_DB_PATH, O_RDWR);
    if(fd == -1){
        perror("open() ");
        return -1;
    }

    int len;

    while(len = read(fd, &account, sizeof(account))){
        if (account.id == account_id)break;
    }

    if( len!=0 ){
        int pos = lseek(fd, -sizeof(account), SEEK_CUR);
        int recordNo = pos/sizeof(account);
        lock(fd, F_WRLCK, sizeof(account), recordNo);
            read(fd, &account, sizeof(account));
            lseek(fd, -sizeof(account), SEEK_CUR);
            account.balance+= amount;
            if(account.balance < 0){
                close(fd);
                return -1;
            }
            write(fd, &account, sizeof(account));
        lock(fd, F_UNLCK, sizeof(account), recordNo);

        close(fd);
        return 0;

    }

    close(fd);
    return -1;
}

int saveUser(struct User * user){

    struct User temp;

    int fd = open(USER_DB_PATH, O_RDWR);
    if(fd == -1){
        perror("open() ");
        return -1;
    }

    int len;


    while(len = read(fd, &temp, sizeof(temp))){
        if (temp.id == user->id)break;
    }

    if( len!=0 ){
        int pos = lseek(fd, -sizeof(temp), SEEK_CUR);
        int recordNo = pos/sizeof(temp);
        lock(fd, F_WRLCK, sizeof(temp), recordNo);
            write(fd, user, sizeof(*user));
            sleep(DELAY);
        lock(fd, F_UNLCK, sizeof(temp), recordNo);

        close(fd);
        return 0;

    }

    close(fd);
    return -1;
}

int deleteUser(int user_id){

    struct User temp;

    int fd = open(USER_DB_PATH, O_RDWR);
    if(fd == -1){
        perror("open() ");
        return -1;
    }

    int len;

    while(len = read(fd, &temp, sizeof(temp))){
        if (temp.id == user_id)break;
    }

    if( len!=0 ){
        temp.id = -1;
        int pos = lseek(fd, -sizeof(temp), SEEK_CUR);
        int recordNo = pos/sizeof(temp);
        lock(fd, F_WRLCK, sizeof(temp), recordNo);
            write(fd, &temp, sizeof(temp));
            sleep(DELAY);
        lock(fd, F_UNLCK, sizeof(temp), recordNo);

        close(fd);
        return 0;

    }

    close(fd);
    return -1;
}


#endif
