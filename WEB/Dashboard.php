<?php
defined('BASEPATH') OR exit('No direct script access allowed');
class Dashboard extends CI_Controller {
  public function liveData() {
    $data = array();
    $snd_msg = "VALUES|0";
    $rcv_msg = "";
    $timeout = 5;
    $socket = stream_socket_client('unix:///var/run/openZmeter.sock', $errorno, $errorstr, $timeout);
    stream_set_timeout($socket, $timeout);
    if(fwrite($socket, $snd_msg)) {
      do {
        $buff = @fread($socket, 8192);
        if(strlen($buff) == 0) break;
        $rcv_msg .= $buff;
      } while (true);
      $data['Params'] = json_decode($rcv_msg);
    }
    $snd_msg = "OSCILLOSCOPE|" . $this->input->post('lastTimestamp');
    $rcv_msg = "";
    $timeout = 5;
    $socket = stream_socket_client('unix:///var/run/openZmeter.sock', $errorno, $errorstr, $timeout);
    stream_set_timeout($socket, $timeout);
    if(fwrite($socket, $snd_msg)) {
      do {
        $buff = @fread($socket, 8192);
        if(strlen($buff) == 0) break;
        $rcv_msg .= $buff;
      } while (true);
      $Tmp = (array)json_decode($rcv_msg);
      $data['wave_v'] = array();
      $data['wave_i'] = array();
      for($i = 0; $i < count($Tmp['points']); $i += 2) { 
        $data['wave_v'][] = $Tmp['points'][$i];
        $data['wave_i'][] = $Tmp['points'][$i+1];
      }
    }
    echo json_encode($data);
  }
  public function ajaxUpdateValues60s() {
    $energyPeriod = $this->input->post('energyperiod');
    if($energyPeriod == 'Day') {
      $agreg = '1m';
      $beginDate = strtotime(date('Y-m-d 00:00:00'));
      $PrevBeginDate = strtotime(date('d.m.Y 00:00:00', strtotime("-1 days")));
    } else if($energyPeriod == 'Week') {
      $agreg = '1h';
      $beginDate = strtotime(date('d-m-Y 00:00:00', strtotime((-date('w') + 1) . ' days')));
      $PrevBeginDate = strtotime(date('d-m-Y 00:00:00', strtotime((-date('w') - 6) . ' days')));       
    } else if($energyPeriod == 'Month') {
      $agreg = '1h';
      $beginDate = strtotime(date('01-m-Y 00:00:00'));
      $PrevBeginDate = strtotime(date('01-m-Y 00:00:00', strtotime('-1 months')));        
    } else if($energyPeriod == 'Year') {
      $agreg = '1h';
      $beginDate = strtotime(date('01-01-Y 00:00:00'));
      $PrevBeginDate = strtotime(date('01-01-Y 00:00:00', strtotime('-1 years')));     
    }
    $query = $this->db->query('SELECT SUM(active_energy) AS energy, SUM(active_energy * price / 1000.0) as budget, LEAST((SELECT MAX(sampletime) + 1 FROM agreg_' . $agreg . '), (SELECT MAX(starttime) * 1000 FROM prices)) / 1000 AS lastcalc FROM agreg_' . $agreg . ', prices WHERE (sampletime / 3600 / 1000) * 3600 = starttime AND starttime >=' . $beginDate)->row();
    $data['current_energy'] = (float)$query->energy;
    $data['current_budget'] = (float)$query->budget;
    $data['budget_lastcalc'] = (int)$query->lastcalc;
    $query = $this->db->query('SELECT SUM(active_energy) as energy FROM agreg_1h, prices WHERE (sampletime / 3600 / 1000) * 3600 = starttime AND starttime <' . $beginDate . ' AND starttime >=' . $PrevBeginDate)->row();      
    $data['prev_energy'] = (float)$query->energy;
    $Today = strtotime(date('Y-m-d 00:00:00'));
    $query = $this->db->query('SELECT SUM(active_energy) as energy FROM agreg_1m WHERE (sampletime / 1000) > ' . $Today)->row();
    $data['today_energy'] = (float)$query->energy;
    $get = $this->input->post('getEvolution');
    if($get == 'true') {
      $from = $this->input->post('lastevolutiontimestamp');
      $query = $this->db->query('SELECT sampletime / 1000 AS sampletime, active_power, fundamental_reactive_power, rms_v, rms_i, power_factor, phi FROM agreg_1m WHERE (sampletime / 1000) > ' . $from . ' AND sampletime >= (SELECT MAX(sampletime) - 86400000 FROM agreg_1m) ORDER BY sampletime ASC');
      foreach($query->result() as $row) {
        $data['evolution_time'][] = $row->sampletime;
        $data['active_power'][] = json_decode($row->active_power);
        $data['fundamental_reactive_power'][] = json_decode($row->fundamental_reactive_power);
        $data['rms_v'][] = json_decode($row->rms_v);
        $data['rms_i'][] = json_decode($row->rms_i);
        $data['power_factor'][] = json_decode($row->power_factor);
        $data['phi'][] = json_decode($row->phi);        
      }
    }
    echo json_encode($data);
  }
  public function ajaxUpdateValues5s() {
    if($this->input->is_ajax_request() == false) return;
    $data = array();
    $to = time();
    $from = $from = $this->input->post('lasttimestamp');
    $from = max($from, $to - 7200);
    $query = $this->db->query('SELECT sampletime / 1000 as sampletime, rms_v, rms_i, active_power, freq FROM agreg_3s WHERE (sampletime / 1000) > ' . $from . ' AND (sampletime / 1000) <= ' . $to . ' ORDER BY sampletime ASC');
    $data['sampletime'] = array();
    foreach($query->result() as $row) {
      $data['sampletime'][] = (int)$row->sampletime;
      $data['rms_v'][] = (float)$row->rms_v;
      $data['rms_i'][] = (float)$row->rms_i;
      $data['freq'][] = (float)$row->freq;
      $data['active_power'][] = (float)$row->active_power;
    }
    $data['aggregation_source'] = '3 seconds';
    $this->load->model('Sysinfo_model');
    $data['sysinfo']['ram'] = $this->Sysinfo_model->getServerMemoryUsage();
    $data['sysinfo']['ssd'] = $this->Sysinfo_model->getServerFilesystemUsage();
    $data['sysinfo']['cpu'] = $this->Sysinfo_model->getServerCpuUsage();
    echo json_encode($data);
  }
  public function ajaxUpdateEnergy() {
    if($this->input->is_ajax_request() == false) return;
    $data = array();
    $to = (int)$this->input->post('to');
    $lastSample = (int)$this->input->post('lastSample');
    $period = (int)$this->input->post('period');
    $update = ($this->input->post('update') == 'true');
    $data['update'] = $update;
    $data['energy_range'] = array();
    if($to == 0) $to = time();
    $from = $to - $period;
    $agreg = $this->input->post('agreg');
    $data['from'] = $from;
    $data['to'] = $to;
    $maxpts = 10000;
    if($agreg == '3s') {
      $from -= 3;
      $to   += 3;
      if(($period / 3)   > $maxpts) $agreg = '1m';
      if(($period / 60)  > $maxpts) $agreg = '10m';
      if(($period / 600) > $maxpts) $agreg = '1h';
    } else if($agreg == '1m') {
      $from -= 60;
      $to   += 60;
      if(($period / 60)  > $maxpts) $agreg = '10m';
      if(($period / 600) > $maxpts) $agreg = '1h';      
    } else if($agreg == '10m') {
      $from -= 600;
      $to   += 600;
      if(($period / 600) > $maxpts) $agreg = '1h';
    } else if($agreg == '1h') {
      $from -= 3600;
      $to   += 3600;
    }
    if($agreg == '3s') {
      $data['aggregation_source'] = '3 seconds';
    } else if($agreg == '1m') {
      $data['aggregation_source'] = '1 minute';
    } else if($agreg == '10m') {
      $data['aggregation_source'] = '10 minutes';
    } else if($agreg == '1h') {
      $data['aggregation_source'] = '1 hour';
    }
    if($update == "true") {
      $query = $this->db->query('SELECT sampletime / 1000 as sampletime, active_energy FROM agreg_' . $agreg . ' WHERE (sampletime / 1000) > ' . $lastSample . ' AND (sampletime / 1000) >= ' . $from . ' AND (sampletime / 1000) <= ' . $to . ' ORDER BY sampletime ASC');
    } else {
      $query = $this->db->query('SELECT sampletime / 1000 as sampletime, active_energy FROM agreg_' . $agreg . ' WHERE (sampletime / 1000) >= ' . $from . ' AND (sampletime / 1000) <= ' . $to . ' ORDER BY sampletime ASC');      
    }
    foreach($query->result() as $row) {
      $data['energy_range'][] = [(int)$row->sampletime, (float)$row->active_energy];
    }
    echo json_encode($data);
  }
  public function index() {
    $this->load->model('event_model');
    $data['numEvents'] = $this->event_model->getNumUnreadEvents($this->session->eventTimestamp);
    $this->load->view('dashboard_view', $data);
  }
}
