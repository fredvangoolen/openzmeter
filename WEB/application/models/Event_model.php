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

class Event_model extends CI_Model {

    public function __construct() {
// Call the CI_Model constructor
        parent::__construct();
    }

    public function getEvents($start, $end = NULL, $limit = NULL, $getRVC = FALSE) {
        $query = $this->db->select('starttime, duration, type, voltage, percent')->from('events')->where('starttime >', $start);
        if ($end != NULL) {
            $query = $query->where('starttime <', $end);
        }
        if (!$getRVC) {
            $query->where('type !=', 'RVC');
        }
        if ($limit != NULL) {
            $query = $query->limit($limit);
        }
        $query = $query->order_by('starttime', 'desc')->get();
        return $query->result();
    }

    public function getNumUnreadEvents($start, $getRVC = FALSE) {
        $query = $this->db->select('count(*) as count', false)->from('events')->where('starttime >', +$start * 1000);
        if (!$getRVC) {
            $query->where('type !=', 'RVC');
        }
        $query = $query->get();
        return $query->row()->count;
    }

    public function getWebEventTimestamp() {
        $query = $this->db->select('extract(epoch from read_events_time) as time', false)->from('users')->where('username', 'admin')->get();
        $time = $query->row()->time;
        return $time;
    }

    public function setWebEventTimestamp($timestamp) {
        $query = $this->db->where('username', 'admin')->set('read_events_time', "to_timestamp(" . ($timestamp / 1000) . ")", false)->update('users');
        return $query == TRUE;
    }

    public function getEventSamples($timestamp) {
        try {
            $query = $this->db->select("samples", false)->from('events')->where('sampletime', $timestamp)->get();
            $samples = $query->row();
            //echo $samples->samples;
            $samples->samples = str_replace("NULL", 'null', $samples->samples);
            $samplesSt = substr($samples->samples, 1, -1); //trim($samples->samples, "{}");
            $samplesSt = '[' . $samplesSt . ']';
            return json_decode($samplesSt);
        } catch (Exception $e) {
            return array();
        }

//        $query = $this->db->from('events')->where('timestamp >', $start);
//        if ($end != NULL) {
//            $query = $query->where('timestamp <', $end);
//        }
//        if ($limit != NULL) {
//            $query = $query->limit($limit);
//        }
//        $query = $query->get();
//        return $query->result();
    }

}
