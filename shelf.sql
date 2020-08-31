-- phpMyAdmin SQL Dump
-- version 5.0.1
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1
-- Generation Time: Aug 31, 2020 at 11:36 AM
-- Server version: 10.4.11-MariaDB
-- PHP Version: 7.4.2

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `shelf`
--

-- --------------------------------------------------------

--
-- Table structure for table `ambient`
--

CREATE TABLE `ambient` (
  `id` int(255) NOT NULL,
  `temperature` tinyint(1) NOT NULL,
  `humidity` tinyint(1) NOT NULL,
  `light` tinyint(1) NOT NULL,
  `time` timestamp NOT NULL DEFAULT current_timestamp() ON UPDATE current_timestamp()
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Dumping data for table `ambient`
--

INSERT INTO `ambient` (`id`, `temperature`, `humidity`, `light`, `time`) VALUES
(1, 23, 70, 5, '2020-08-31 09:35:04');

-- --------------------------------------------------------

--
-- Table structure for table `commands`
--

CREATE TABLE `commands` (
  `id` int(255) NOT NULL,
  `lamp` tinyint(1) NOT NULL,
  `fan` tinyint(1) NOT NULL,
  `buzzer` tinyint(1) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Dumping data for table `commands`
--

INSERT INTO `commands` (`id`, `lamp`, `fan`, `buzzer`) VALUES
(1, 0, 0, 1);

-- --------------------------------------------------------

--
-- Table structure for table `products`
--

CREATE TABLE `products` (
  `id` int(255) NOT NULL,
  `sold` tinyint(1) NOT NULL,
  `price` int(255) NOT NULL,
  `name` varchar(20) NOT NULL,
  `image` varchar(64) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Dumping data for table `products`
--

INSERT INTO `products` (`id`, `sold`, `price`, `name`, `image`) VALUES
(2, 0, 13, 'Tea Glass', 'tea glass.jpg'),
(10, 0, 4, 'Shot Glass', 'shot glass.jpg'),
(34, 1, 7, 'Tiny Cup', 'ceramic tiny cup.jpg'),
(38, 0, 23, 'Blue Bowl', 'Blue Bowl.jpg'),
(42, 0, 12, 'Ceramic Plate', 'White Plate.jpg'),
(59, 0, 18, 'Flower Plate', 'Flower Plate.jpg'),
(71, 0, 20, 'Spoon & Fork', 'Spoon & Fork.jpg'),
(72, 0, 27, 'Ceramic Bowl', 'bowl.jpg'),
(75, 0, 12, 'Square Plate', 'square plate.jpg'),
(86, 0, 16, 'Scoop', 'Scoop.jpg'),
(115, 0, 18, 'Salt Shaker', 'Salt Shaker.jpg'),
(147, 0, 15, 'Ceramic Cup', 'ceramic cup.jpg');

-- --------------------------------------------------------

--
-- Table structure for table `sensors`
--

CREATE TABLE `sensors` (
  `id` int(255) NOT NULL,
  `status` tinyint(1) NOT NULL,
  `unit_id` int(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Dumping data for table `sensors`
--

INSERT INTO `sensors` (`id`, `status`, `unit_id`) VALUES
(1, 1, 1),
(2, 1, 1),
(3, 1, 1),
(4, 1, 1),
(5, 4, 2),
(6, 1, 3),
(7, 1, 3),
(8, 1, 3),
(9, 1, 3),
(10, 1, 3),
(11, 3, 4),
(12, 3, 5);

-- --------------------------------------------------------

--
-- Table structure for table `units`
--

CREATE TABLE `units` (
  `id` int(255) NOT NULL,
  `product_id` int(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Dumping data for table `units`
--

INSERT INTO `units` (`id`, `product_id`) VALUES
(3, 2),
(1, 10),
(2, 34),
(5, 72),
(4, 75);

--
-- Indexes for dumped tables
--

--
-- Indexes for table `ambient`
--
ALTER TABLE `ambient`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `commands`
--
ALTER TABLE `commands`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `products`
--
ALTER TABLE `products`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `sensors`
--
ALTER TABLE `sensors`
  ADD PRIMARY KEY (`id`),
  ADD KEY `FOREIGN` (`unit_id`) USING BTREE;

--
-- Indexes for table `units`
--
ALTER TABLE `units`
  ADD PRIMARY KEY (`id`),
  ADD KEY `FOREIGN` (`product_id`) USING BTREE;

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `ambient`
--
ALTER TABLE `ambient`
  MODIFY `id` int(255) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;

--
-- AUTO_INCREMENT for table `commands`
--
ALTER TABLE `commands`
  MODIFY `id` int(255) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;

--
-- AUTO_INCREMENT for table `products`
--
ALTER TABLE `products`
  MODIFY `id` int(255) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=150;

--
-- AUTO_INCREMENT for table `sensors`
--
ALTER TABLE `sensors`
  MODIFY `id` int(255) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=13;

--
-- AUTO_INCREMENT for table `units`
--
ALTER TABLE `units`
  MODIFY `id` int(255) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=16;

--
-- Constraints for dumped tables
--

--
-- Constraints for table `sensors`
--
ALTER TABLE `sensors`
  ADD CONSTRAINT `sensors_ibfk_1` FOREIGN KEY (`unit_id`) REFERENCES `units` (`id`) ON UPDATE CASCADE;

--
-- Constraints for table `units`
--
ALTER TABLE `units`
  ADD CONSTRAINT `test` FOREIGN KEY (`product_id`) REFERENCES `products` (`id`);
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
