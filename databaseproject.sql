
create table publishers(
publisher_id INT AUTO_INCREMENT PRIMARY KEY,
first_name varchar(60),
addres varchar(100)
);
create table Categories (
Category_ID INT AUTO_INCREMENT PRIMARY KEY,
Category_name varchar(50)
);
create table authors (
Author_ID INT AUTO_INCREMENT PRIMARY KEY,
 Author_first_name varchar(60),
 Nationality varchar(60)
);
create table books (
Book_ID INT AUTO_INCREMENT PRIMARY KEY,
Book_name varchar(60),
Availability varchar(60),
Publisher_id INT, 
Author_ID INT,
Category_ID INT,
FOREIGN KEY (publisher_id) REFERENCES publishers(publisher_id),
FOREIGN KEY (Author_ID) REFERENCES authors(Author_ID),
FOREIGN KEY (Category_ID) REFERENCES Categories(Category_ID)
);
ALTER TABLE authors
ADD COLUMN last_name VARCHAR(60);
ALTER TABLE publishers
CHANGE COLUMN first_name publisher_name VARCHAR(60);
ALTER TABLE publishers
CHANGE COLUMN addres country VARCHAR(60);

 INSERT INTO authors (Author_first_name, last_name, Nationality) VALUES
    ('Naguib', 'Mahfouz', 'Egyptian'),
    ('Taha', 'Hussein', 'Egyptian'),
    ('Alaa', 'Al Aswany', 'Egyptian'),
    ('Ahdaf', 'Soueif', 'Egyptian'),
    ('Yusuf', 'Idris', 'Egyptian'),
    ('Tawfiq', 'al-Hakim', 'Egyptian'),
    ('Radwa', 'Ashour', 'Egyptian'),
    ('Sonallah', 'Ibrahim', 'Egyptian'),
    ('Bahaa', 'Taher', 'Egyptian'),
    ('Nawal', 'El Saadawi', 'Egyptian');     
  
   INSERT INTO publishers (publisher_name, country) VALUES
    ('Dar El Shorouk', 'Egypt'),
    ('American University in Cairo Press', 'Egypt'),
    ('Al-Dar Al-Masriah Al-Lubnaniah', 'Egypt' ),
    ('Dar Al-Adab', 'Lebanon' ),
    ('Bloomsbury', 'UK' ),
    ('Doubleday', 'US'),
    ('Pantheon Books', 'US'),
    ('Zed Books', 'UK'),
    ('Hoover Institution Press', 'US'),
    ('Arab Scientific Publishers', 'Lebanon');
    select *
    from publishers;
    INSERT INTO Categories (Category_name) VALUES
    -- Fiction Genres
    ('Literary Fiction'),
    ('Historical Fiction'),
    ('Science Fiction'),
    ('Fantasy'),
    ('Mystery'),
    ('Thriller'),
    ('Romance'),
    ('Horror'),
    
    -- Arabic Literature Specific
    ('Arabic Poetry'),
    ('Islamic Literature'),
    ('Nahda Literature'),  -- Arab Renaissance period
    ('Modern Arabic Novel'),
    ('Classic Arabic Literature'),
    
    -- Non-Fiction
    ('Biography'),
    ('History'),
    ('Politics'),
    ('Philosophy'),
    ('Religion'),
    ('Self-Help'),
    
    -- Special Collections
    ('Nobel Prize Winners'),
    ('Egyptian Authors'),
    ('Post-Colonial Literature'),
    ('Feminist Literature'),
    
    -- Age Categories
    ('Children''s Literature'),
    ('Young Adult'),
    ('Adult Fiction'),
    
    -- Formats
    ('Short Stories'),
    ('Novellas'),
    ('Drama/Plays'),
    ('Essays');
    select*
    from publishers;
    delete from books
    where Book_ID =3;
    ALTER TABLE books
MODIFY COLUMN Book_name varchar(60) NOT NULL;

ALTER TABLE books
MODIFY COLUMN Availability varchar(60) NOT NULL;

ALTER TABLE books
MODIFY COLUMN Publisher_id INT NOT NULL;

ALTER TABLE books
MODIFY COLUMN Author_ID INT NOT NULL;

ALTER TABLE books
MODIFY COLUMN Category_ID INT NOT NULL;
create table users(
user_id INT AUTO_INCREMENT PRIMARY KEY,
first_name varchar(50),
last_name varchar(50),
Mobile_Number int not null,
Role varchar(50)
);
ALTER TABLE users
ADD COLUMN Password VARCHAR(255);
ALTER TABLE users
ADD COLUMN username VARCHAR(255) unique;

create table Issued_Books(
Issue_ID INT AUTO_INCREMENT PRIMARY KEY,
Due_Date date,
Issue_date date,
Book_ID int,
user_id int ,
FOREIGN KEY (Book_ID) REFERENCES books(Book_ID) on delete cascade,
FOREIGN KEY (user_id) REFERENCES users(user_id)on delete cascade

);
create table Book_Reviews (
Review_ID INT AUTO_INCREMENT PRIMARY KEY,
Comment VARCHAR(50),
Book_ID int,
FOREIGN KEY (Book_ID) REFERENCES books(Book_ID) on delete cascade
);
ALTER TABLE publishers DROP COLUMN country;
ALTER TABLE authors DROP COLUMN Nationality;
select *
from books;
drop table Book_Reviews;

