-- SQL script for creating the database tables
-- Authors: Tomas Meszaros, David Molnar
-- Date: 2011-11-01
-- Encoding: UTF-8
-- ------------------------------------

USE `xmolna02`;

DROP TABLE IF EXISTS `Ucebna`;
CREATE TABLE IF NOT EXISTS `Ucebna` (
  `id_miest` CHAR(4) NOT NULL,
  `typ_miest` VARCHAR(30) NOT NULL,
  `kapacita` INT NOT NULL,
  `spec_vyb` VARCHAR(100),
  PRIMARY KEY (`id_miest`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
  
DROP TABLE IF EXISTS `Zamestnanec`;
CREATE TABLE IF NOT EXISTS `Zamestnanec` (
  `r_cislo` CHAR(11) NOT NULL,
  `role` INT NOT NULL,
  `username` VARCHAR(20) NOT NULL,
  `heslo` CHAR(32) NOT NULL,
  `meno` VARCHAR(50) NOT NULL,
  `priezvisko` VARCHAR(50) NOT NULL,
  `zobraz_meno` VARCHAR(100) NOT NULL,
  `titul` VARCHAR(50),
  `prac_pomer` VARCHAR(50) NOT NULL,
  PRIMARY KEY (`r_cislo`),
  UNIQUE (`username`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
  
DROP TABLE IF EXISTS `Predmet`;
CREATE TABLE IF NOT EXISTS `Predmet` (
  `skratka_predmetu` VARCHAR(4) NOT NULL,
  `nazov` VARCHAR(50) NOT NULL,
  `ustav` VARCHAR(100),
  PRIMARY KEY (`skratka_predmetu`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
  
DROP TABLE IF EXISTS `Rezervacia`;
CREATE TABLE IF NOT EXISTS `Rezervacia` (
  `c_rezer` INT(11) NOT NULL AUTO_INCREMENT,
  `cas_zac` DATETIME NOT NULL,
  `cas_ukon` DATETIME NOT NULL,
  `typ_udal` VARCHAR(50),
  `r_cislo` CHAR(11) NOT NULL,
  `id_miest` CHAR(4) NOT NULL,
  PRIMARY KEY (`c_rezer`),
  FOREIGN KEY (`r_cislo`) REFERENCES `Zamestnanec` ON DELETE CASCADE,
  FOREIGN KEY (`id_miest`) REFERENCES `Ucebna`
) ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=0;
  
DROP TABLE IF EXISTS `Vyuka`;
CREATE TABLE IF NOT EXISTS `Vyuka` (
  `c_vyuky` INT(11) NOT NULL AUTO_INCREMENT,
  `typ_vyuky` VARCHAR(50),
  `cas_zac` DATETIME NOT NULL,
  `cas_ukon` DATETIME NOT NULL,
  `pocet_reg_ziak` INT DEFAULT 0,
  `stud_obor` VARCHAR(50) NOT NULL,
  `rocnik` INT NOT NULL,
  `id_miest` CHAR(4) NOT NULL,
  `skratka_predmetu` VARCHAR(4) NOT NULL,
  `r_cislo` CHAR(11) NOT NULL,
  PRIMARY KEY (`c_vyuky`),
  FOREIGN KEY (`id_miest`) REFERENCES `Ucebna`,
  FOREIGN KEY (`skratka_predmetu`) REFERENCES `Predmet`,
  FOREIGN KEY (`r_cislo`) REFERENCES `Zamestnanec` ON DELETE CASCADE
) ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=0;
