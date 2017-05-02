-- SQL script to fill database with data
-- Authors: Tomas Meszaros, David Molnar
-- Datum: 2011-11-01
-- Encoding: UTF-8
-- ------------------------------------

USE `xmolna02`;

INSERT INTO `Ucebna` VALUES ('E112', 'Poslucháreň', 156, NULL);
INSERT INTO `Ucebna` VALUES ('E104', 'Poslucháreň', 72, NULL);
INSERT INTO `Ucebna` VALUES ('D105', 'Poslucháreň', 300, NULL);
INSERT INTO `Ucebna` VALUES ('D206', 'Poslucháreň', 154, NULL);
INSERT INTO `Ucebna` VALUES ('L203', 'Počítačové laboratorium', 20,
                           '20 PC, sluchátka');
INSERT INTO `Ucebna` VALUES ('N103', 'Počítačové laboratorium', 20, '20 PC');
INSERT INTO `Ucebna` VALUES ('O203', 'Počítačové laboratorium', 20, NULL);
INSERT INTO `Ucebna` VALUES ('O204', 'Sieťové laboratorium', 20, NULL);

INSERT INTO `Zamestnanec` VALUES ('700315/1002', 1, 'admin', MD5('admin'), 'FIT', 'Admin', 'admin',
                                  '', 'Administrator');
INSERT INTO `Zamestnanec` VALUES ('700314/1002', 2, 'maximilian', MD5('maximilian'), 'Jan', 'Maxmilián', 
                                  'Jan Maximilián', 'Prof. Ing.', 'Profesor');
INSERT INTO `Zamestnanec` VALUES ('910609/0279', 2, 'bidlo', MD5('bidlo'), 'Michal', 'Bidlo', 
                                  'Michal Bidlo', 'Ing.', 'Odborný asistent');
INSERT INTO `Zamestnanec` VALUES ('820812/2615', 2, 'sekanina', MD5('sekanina'), 'Lukáš', 'Sekanina', 
                                  'Lukáš Sekanina', 'Doc. Ing.', 'Zástupca vedúceho ústavu');
INSERT INTO `Zamestnanec` VALUES ('861128/4946', 2, 'kotasek', MD5('kotasek'), 'Zdeněk', 'Kotásek', 
                                  'Zdeněk Kotásek', 'Doc. Ing.', 'Vedúci ústavu');
INSERT INTO `Zamestnanec` VALUES ('850315/4924', 2, 'holik', MD5('holik'), 'Lukáš', 'Holík',
                                  'Lukáš Holík', 'Mgr.', 'Asistent');
INSERT INTO `Zamestnanec` VALUES ('680816/7476', 2, 'rogalewicz', MD5('rogalewicz'), 'Adam', 'Rogalewicz', 
                                  'Adam Rogalewicz', 'Mgr.', 'Odborný asistent');
INSERT INTO `Zamestnanec` VALUES ('830604/1293', 2, 'zboril', MD5('zboril'), 'František', 'Zbořil', 
                                  'František Zbořil', 'Ing.', 'Odborný asistent');

INSERT INTO `Predmet` VALUES ('IZP', 'Základy programovania', 'UIFS');
INSERT INTO `Predmet` VALUES ('IPZ', 'Periferné zariadenie', 'UPSY');
INSERT INTO `Predmet` VALUES ('IJC', 'Jazyk C', 'UITS');
INSERT INTO `Predmet` VALUES ('IAL', 'Algoritmy', 'UIFS');
INSERT INTO `Predmet` VALUES ('IDS', 'Databázové systémy', 'UIFS');
INSERT INTO `Predmet` VALUES ('IOS', 'Operačné systémy', 'UITS');
INSERT INTO `Predmet` VALUES ('SIN', 'Inteligentné systémy', 'UITS');

INSERT INTO `Rezervacia` (cas_zac, cas_ukon, typ_udal, r_cislo, id_miest) 
VALUES ('2010-03-18 08:00', '2010-03-18 10:00', 'Konzultácia', '700314/1002', 'E112');
INSERT INTO `Rezervacia` (cas_zac, cas_ukon, typ_udal, r_cislo, id_miest) 
VALUES ('2010-03-01 12:00', '2010-03-01 14:00', 'Demonstračné cvičenie', '850315/4924', 'D206');
INSERT INTO `Rezervacia` (cas_zac, cas_ukon, typ_udal, r_cislo, id_miest) 
VALUES ('2010-02-16 10:00', '2010-02-16 11:00', 'Seminár', '700314/1002', 'N103');

INSERT INTO `Vyuka` (typ_vyuky, cas_zac, cas_ukon, pocet_reg_ziak, 
                   stud_obor, rocnik, id_miest, skratka_predmetu, r_cislo)
VALUES ('Prednáška', '2010-03-08 07:00', '2010-03-08 10:00', 100, 
'Informačné technológie', 1, 'D105', 'IOS', '820812/2615');
INSERT INTO `Vyuka` (typ_vyuky, cas_zac, cas_ukon, pocet_reg_ziak, 
                   stud_obor, rocnik, id_miest, skratka_predmetu, r_cislo)
VALUES ('Prednáška', '2010-03-09 13:00', '2010-03-09 16:00', 50, 
'Informačné systémy', 1, 'D206', 'SIN', '861128/4946');
INSERT INTO `Vyuka` (typ_vyuky, cas_zac, cas_ukon, pocet_reg_ziak, 
                   stud_obor, rocnik, id_miest, skratka_predmetu, r_cislo)
VALUES ('Seminár', '2010-03-10 13:00', '2010-03-10 15:00', 80,
 'Informačné technologie', 2, 'E112', 'IAL', '700314/1002');

COMMIT;
