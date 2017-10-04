<?php

/*
 * Copyright (C) 2015 Pedro Sánchez Alguacil
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

class Data_model extends CI_Model {

    public function __construct() {
        // Call the CI_Model constructor
        parent::__construct();
    }

    public function getMainValues() {
        //$this->influxdb->setTimePrecision('ms');
        //$db = $this->influxdb->getDatabase();
        $res = new stdClass();
        $query = $this->db->select('rms_v as voltage_rms, rms_i as current_rms, active_power as power_p, freq', false)->from('agreg_3s')->where('sampletime >', strtotime("-9 second") * 1000)->limit(1)->get();
        $row = $query->row();
        if ($row != null) {
            $res->voltage_rms = $row->voltage_rms;
            $res->current_rms = $row->current_rms;
            $res->freq = $row->freq;
            $res->power_p = $row->power_p;
        }
        $quer = $this->db->select('SUM(active_energy) as energy_w' . false)->where('sampletime >', strtotime('today') * 1000)->get('agreg_3s');
        $row2 = $quer->row();
        if ($row2 != null) {
            $res->energy_w = $row2->energy_w;
        }
        return $res;
    }

    public function getRangedValues($metric, $startDate, $endDate) {
        //$this->influxdb->setTimePrecision('ms');
        //$db = $this->influxdb->getDatabase();
        $ranges = array('agreg_10c', 'agreg_3s', 'agreg_1m', '"agreg_10m', 'agreg_1h');
        $range = $endDate - $startDate;
        $increment = 0;
        if ($range <= 30000) {//Less than 1m30s => max. 150 points of 10c
            $rangeIndex = 0;
            $increment = 1000;
        } else if ($range <= 600000) {//Less than 10 minutes => max. 200 points of 3s
            $rangeIndex = 1;
            $increment = 6000;
        } else if ($range <= 7200000) {//Less than 2 hour => max. 120 points of 1m
            $rangeIndex = 2;
            $increment = 60000;
        } else if ($range <= 86400000) {//Less than 1 day => max. 144 points of 10m
            $rangeIndex = 3;
            $increment = 600000;
        } else if ($range <= 864000000) {//Less than 10 day => max. 240 points of 1h
            $rangeIndex = 4;
            $increment = 3600000;
        } else {//Points of 1d
            $increment = 86400000;
            $rangeIndex = 4;
            //GROUP BY DAY??
        }
        $startDate -= $increment;
        $endDate += $increment;
        $from = $ranges[$rangeIndex];
        $op = '';
        switch ($metric) {
            case 'active_power':
              break;
            case 'fundamental_reactive_power':
            case 'active_energy':
            //$op = '/1000';
        }
        try {
            $query = $this->db->select("sampletime as time, " . $this->db->escape_str($metric) . "{$op} as value", false)->from($from)->where('sampletime >', $startDate)->where('sampletime <', $endDate)->order_by('sampletime', 'asc')->get();
            return $query->result();
        } catch (Exception $e) {
            return array();
        }
    }

    public function getSamples($metric, $startDate, $endDate) {
        $this->influxdb->setTimePrecision('ms');
        $db = $this->influxdb->getDatabase();
        $increment = 1000; //1 ms
        $startDate = number_format($startDate - $increment, 0, "", "");
        $serieName = '"rp_samples".' . $metric . '_samples';
        //echo "SELECT time, value FROM {$serieName} where time > {$startDate}us AND time < {$endDate}us;";
        $query = "SELECT time, value FROM {$serieName} where time > {$startDate}ms";
        if ($endDate != -1) {
            $endDate = number_format($endDate + $increment, 0, "", "");
            $query .= " AND time < {$endDate}ms";
        }
        try {
            $res = $db->query($query);
        } catch (Exception $e) {
            return array();
        }
        if (isset($res[$metric . '_samples'])) {
            return $res[$metric . '_samples'];
        } else {
            return array();
        }
    }

    public function getHarmonics($metric) {
        $now = strtotime('now -5 second') * 1000;
        //$query = $this->db->query('SELECT ' . $metric . ' AS result FROM agreg_3s WHERE sampletime > ' . $now . 'ORDER BY sampletime DESC LIMIT 1');
        $query = $this->db->select($metric . ' as harmonics, freq')->where('sampletime >', $now)->limit(1)->order_by('sampletime', 'desc')->get('agreg_3s');
        $harmonics = new stdClass();
        $harmonics->time = $now;
        $harmonics->freq = $query->row()->freq;
        $values = explode(',', trim($query->row()->harmonics, "{}"));
        for ($i = 0; $i < count($values); $i++) {
            $values[$i] = (float) $values[$i];
        }
        $harmonics->values = $values;
        return $harmonics;
    }

    public function getHarmonicsInfo() {
        $now = strtotime('now -5 second') * 1000;
        //$query = $this->db->query('SELECT ' . $metric . ' AS result FROM agreg_3s WHERE sampletime > ' . $now . 'ORDER BY sampletime DESC LIMIT 1');
        $query = $this->db->select('harmonics_v, harmonics_i, power_factor, phi, thd_i * 100 as thd_i, thd_v * 100 as thd_v', false)->where('sampletime >', $now)->limit(1)->order_by('sampletime', 'desc')->get('agreg_3s');
        $res = $query->row();
        $data = new stdClass();
        $data->time = $now;
        $data->power_factor = $res->power_factor;
        $data->phi = $res->phi;
        $data->thd_v = $res->thd_v;
        $data->thd_i = $res->thd_i;
        $values = explode(',', trim($res->harmonics_v, "{}"));
        for ($i = 0; $i < count($values); $i++) {
            $values[$i] = (float) $values[$i];
        }
        $data->harm_voltage = $values;
        $values = explode(',', trim($res->harmonics_i, "{}"));
        for ($i = 0; $i < count($values); $i++) {
            $values[$i] = (float) $values[$i];
        }
        $data->harm_current = $values;
        return $data;
    }

}
