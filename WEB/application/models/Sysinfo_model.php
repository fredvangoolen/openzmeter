<?php

/*
 * Copyright (C) 2017 Eduardo Viciana Gamez
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

class Sysinfo_model extends CI_Model {
  public function __construct() {
    // Call the CI_Model constructor
    parent::__construct();
  }
  function getServerMemoryUsage() {
    $memoryTotal = null;
    $memoryFree = null;
    $memoryBuff = null;
    if (is_readable("/proc/meminfo")) {
      $stats = @file_get_contents("/proc/meminfo");
      if ($stats !== false) {
        $stats = str_replace(array("\r\n", "\n\r", "\r"), "\n", $stats);
        $stats = explode("\n", $stats);
        foreach ($stats as $statLine) {
          $statLineData = explode(":", trim($statLine));
          if (count($statLineData) == 2 && trim($statLineData[0]) == "MemTotal") {
            $memoryTotal = trim($statLineData[1]);
            $memoryTotal = explode(" ", $memoryTotal);
            $memoryTotal = $memoryTotal[0];
          }
          if (count($statLineData) == 2 && trim($statLineData[0]) == "MemFree") {
            $memoryFree = trim($statLineData[1]);
            $memoryFree = explode(" ", $memoryFree);
            $memoryFree = $memoryFree[0];
          }
          if (count($statLineData) == 2 && trim($statLineData[0]) == "Cached") {
            $memoryBuff = trim($statLineData[1]);
            $memoryBuff = explode(" ", $memoryBuff);
            $memoryBuff = $memoryBuff[0];
          }
        }
      }
    }
    if (is_null($memoryTotal) || is_null($memoryFree) || is_null($memoryBuff)) {
      return null;
    } else {
      $values = array();
      $values[] = (float)($memoryFree + $memoryBuff);
      $values[] = (float)$memoryTotal;
      return $values;
    }
  }
  function getServerFilesystemUsage() {
    $values = array();
    $values[] = (float)disk_free_space('/');
    $values[] = (float)disk_total_space('/');
    return $values;
  }
  function getServerCpuUsage() {
    $numCpus = 1;
    if(is_file('/proc/cpuinfo')) {
      $cpuinfo = file_get_contents('/proc/cpuinfo');
      preg_match_all('/^processor/m', $cpuinfo, $matches);
      $numCpus = count($matches[0]);
    }
    $load = sys_getloadavg();
    return min(100, (100 * $load[0]) / $numCpus);
  }  
}
