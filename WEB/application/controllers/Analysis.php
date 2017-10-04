<?php
defined('BASEPATH') OR exit('No direct script access allowed');
class Analysis extends CI_Controller {
  public function index() {
    redirect('analysis/voltage');
  }
  public function ajaxUpdateVoltage() {
    if($this->input->is_ajax_request() == false) return;
    $data = array();
    $to = (int)$this->input->post('to');
    $lastSample = (int)$this->input->post('lastSample');
    $period = (int)$this->input->post('period');
    $update = ($this->input->post('update') == 'true');
    $data['update'] = $update;
    $data['voltage_range'] = array();
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
      $query = $this->db->query('SELECT sampletime / 1000 as sampletime, rms_v FROM agreg_' . $agreg . ' WHERE (sampletime / 1000) > ' . $lastSample . ' AND (sampletime / 1000) >= ' . $from . ' AND (sampletime / 1000) <= ' . $to . ' ORDER BY sampletime ASC');
    } else {
      $query = $this->db->query('SELECT sampletime / 1000 as sampletime, rms_v FROM agreg_' . $agreg . ' WHERE (sampletime / 1000) >= ' . $from . ' AND (sampletime / 1000) <= ' . $to . ' ORDER BY sampletime ASC');      
    }
    foreach($query->result() as $row) {
      $data['voltage_range'][] = [(int)$row->sampletime, (float)$row->rms_v];
    }
    echo json_encode($data);
  }
  public function ajaxUpdateValues60sVoltage() {
    if(!$this->input->is_ajax_request()) return;
    $data = array();
    $get = $this->input->post('harmonics');
    $lasthour = time() - 3600;
    $query = $this->db->query('SELECT SQRT(AVG(POW(rms_v, 2.0))) AS hour_rms, MAX(rms_v) AS hour_max, MIN(rms_v) as hour_min FROM agreg_1m WHERE (sampletime / 1000) >= ' . $lasthour)->row();
    $data['hour_rms'] = (float)$query->hour_rms;
    $data['hour_min'] = (float)$query->hour_min;
    $data['hour_max'] = (float)$query->hour_max;
    $lastday = time() - 86400;
    $query = $this->db->query('SELECT SQRT(AVG(POW(rms_v, 2.0))) AS day_rms, MAX(rms_v) AS day_max, MIN(rms_v) as day_min FROM agreg_10m WHERE (sampletime / 1000) >= ' . $lastday)->row();
    $data['day_rms'] = (float)$query->day_rms;
    $data['day_min'] = (float)$query->day_min;
    $data['day_max'] = (float)$query->day_max;
    $Today = strtotime(date('Y-m-d 00:00:00'));      
    $query = $this->db->query('SELECT SUM(active_energy) as energy FROM agreg_10m WHERE (sampletime / 1000) >' . $Today . ' AND ((sampletime / 1000) - 86400) <= ' . $Today)->row();
    $data['today_energy'] = (float)$query->energy;
    $from = $this->input->post('lastevolutiontimestamp');
    if($get == 'true') {
      $query = $this->db->query('SELECT sampletime / 1000 AS sampletime, array_to_json(harmonics_v) AS harmonics_v FROM (SELECT sampletime, harmonics_v FROM agreg_1m WHERE (sampletime / 1000) > ' . $from . ' AND sampletime >= (SELECT MAX(sampletime) - 43200000 FROM agreg_1m)) AS Table1 ORDER BY sampletime ASC');
      foreach($query->result() as $row) {
        $data['evolution_time'][] = $row->sampletime;
        $data['evolution_v'][] = json_decode($row->harmonics_v);
      }
    }
    echo json_encode($data);
  }
  public function ajaxUpdateValues5sVoltage() {
    if(!$this->input->is_ajax_request()) return;
    $data = array();
    $from = time() - 20;
    $row = $this->db->query('SELECT sampletime, rms_v, rms_i, active_power, freq, array_to_json(harmonics_v) AS harmonics_v FROM ' .
                            '(SELECT sampletime / 1000 as sampletime, rms_v, rms_i, active_power, freq, harmonics_v FROM agreg_3s WHERE (sampletime / 1000) > ' . $from . ' ORDER BY sampletime DESC LIMIT 1) AS Table1')->row();
    $data['sampletime'] = (int)$row->sampletime;
    $data['rms_v'] = (float)$row->rms_v;
    $data['rms_i'] = (float)$row->rms_i;
    $data['freq'] = (float)$row->freq;
    $data['active_power'] = (float)$row->active_power;
    
    if($this->input->post('paused') === "false") {
      $data['harmonics_v'] = json_decode($row->harmonics_v);
      $snd_msg = "FFT_V|" . $this->input->post('lastTimestamp');
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
        $data['fft_v'] = json_decode($rcv_msg);
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
        $data['wave_v'] = json_decode($rcv_msg);
      }
    }
    $this->load->model('Sysinfo_model');
    $data['sysinfo']['ram'] = $this->Sysinfo_model->getServerMemoryUsage();
    $data['sysinfo']['ssd'] = $this->Sysinfo_model->getServerFilesystemUsage();
    $data['sysinfo']['cpu'] = $this->Sysinfo_model->getServerCpuUsage();
    echo json_encode($data);
  }
  public function voltage() {
    $this->load->model('event_model');
    $data['events'] = $this->event_model->getEvents(strtotime('today') * 1000, strtotime('today + 1 day') * 1000, NULL, TRUE);
    $data['numEvents'] = $this->event_model->getNumUnreadEvents($this->session->eventTimestamp);
    $this->load->view('analysis/voltage_view', $data);
  }
  public function ajaxUpdateCurrent() {
    if($this->input->is_ajax_request() == false) return;
    $data = array();
    $to = (int)$this->input->post('to');
    $lastSample = (int)$this->input->post('lastSample');
    $period = (int)$this->input->post('period');
    $update = ($this->input->post('update') == 'true');
    $data['update'] = $update;
    $data['current_range'] = array();
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
      $query = $this->db->query('SELECT sampletime / 1000 as sampletime, rms_i FROM agreg_' . $agreg . ' WHERE (sampletime / 1000) > ' . $lastSample . ' AND (sampletime / 1000) >= ' . $from . ' AND (sampletime / 1000) <= ' . $to . ' ORDER BY sampletime ASC');
    } else {
      $query = $this->db->query('SELECT sampletime / 1000 as sampletime, rms_i FROM agreg_' . $agreg . ' WHERE (sampletime / 1000) >= ' . $from . ' AND (sampletime / 1000) <= ' . $to . ' ORDER BY sampletime ASC');      
    }
    foreach($query->result() as $row) {
      $data['current_range'][] = [(int)$row->sampletime, (float)$row->rms_i];
    }
    echo json_encode($data);
  }
  public function ajaxUpdateValues60sCurrent() {
    if(!$this->input->is_ajax_request()) return;
    $data = array();
    $get = $this->input->post('harmonics');
    $lasthour = time() - 3600;
    $query = $this->db->query('SELECT SQRT(AVG(POW(rms_i, 2.0))) AS hour_rms, MAX(rms_i) AS hour_max, MIN(rms_i) as hour_min FROM agreg_1m WHERE (sampletime / 1000) >= ' . $lasthour)->row();
    $data['hour_rms'] = (float)$query->hour_rms;
    $data['hour_min'] = (float)$query->hour_min;
    $data['hour_max'] = (float)$query->hour_max;
    $lastday = time() - 86400;
    $query = $this->db->query('SELECT SQRT(AVG(POW(rms_i, 2.0))) AS day_rms, MAX(rms_i) AS day_max, MIN(rms_i) as day_min FROM agreg_10m WHERE (sampletime / 1000) >= ' . $lastday)->row();
    $data['day_rms'] = (float)$query->day_rms;
    $data['day_min'] = (float)$query->day_min;
    $data['day_max'] = (float)$query->day_max;
    $Today = strtotime(date('Y-m-d 00:00:00'));      
    $query = $this->db->query('SELECT SUM(active_energy) as energy FROM agreg_10m WHERE (sampletime / 1000) >' . $Today . ' AND ((sampletime / 1000) - 86400) <= ' . $Today)->row();
    $data['today_energy'] = (float)$query->energy;
    $from = $this->input->post('lastevolutiontimestamp');
    if($get == 'true') {    
      $query = $this->db->query('SELECT sampletime / 1000 AS sampletime, array_to_json(harmonics_i) AS harmonics_i FROM (SELECT sampletime, harmonics_i FROM agreg_1m WHERE (sampletime / 1000) > ' . $from . ' AND sampletime >= (SELECT MAX(sampletime) - 43200000 FROM agreg_1m)) AS Table1 ORDER BY sampletime ASC');
      foreach($query->result() as $row) {
        $data['evolution_time'][] = $row->sampletime;
        $data['evolution_i'][] = json_decode($row->harmonics_i);
      }
    }
    echo json_encode($data);
  }
  public function ajaxUpdateValues5sCurrent() {
    if(!$this->input->is_ajax_request()) return;
    $data = array();
    $from = time() - 20;
    $row = $this->db->query('SELECT sampletime, rms_v, rms_i, active_power, freq, array_to_json(harmonics_i) AS harmonics_i FROM ' .
                            '(SELECT sampletime / 1000 as sampletime, rms_v, rms_i, active_power, freq, harmonics_i FROM agreg_3s WHERE (sampletime / 1000) > ' . $from . ' ORDER BY sampletime DESC LIMIT 1) AS Table1')->row();
    $data['sampletime'] = (int)$row->sampletime;
    $data['rms_v'] = (float)$row->rms_v;
    $data['rms_i'] = (float)$row->rms_i;
    $data['freq'] = (float)$row->freq;
    $data['active_power'] = (float)$row->active_power;
    
    if($this->input->post('paused') === "false") {
      $data['harmonics_i'] = json_decode($row->harmonics_i);
      $snd_msg = "FFT_I|" . $this->input->post('lastTimestamp');
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
        $data['fft_i'] = json_decode($rcv_msg);
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
        $data['wave_i'] = json_decode($rcv_msg);
      }
    }
    $this->load->model('Sysinfo_model');
    $data['sysinfo']['ram'] = $this->Sysinfo_model->getServerMemoryUsage();
    $data['sysinfo']['ssd'] = $this->Sysinfo_model->getServerFilesystemUsage();
    $data['sysinfo']['cpu'] = $this->Sysinfo_model->getServerCpuUsage();
    echo json_encode($data);
  }
  public function current() {
    $this->load->model('event_model');
    $data['events'] = array(); //$this->event_model->getEvents(strtotime('today') * 1000, strtotime('today + 1 day') * 1000);
    $data['numEvents'] = $this->event_model->getNumUnreadEvents($this->session->eventTimestamp);
    $this->load->view('analysis/current_view', $data);
    }
  public function ajaxUpdateEnergyPower() {
    if($this->input->is_ajax_request() == false) return;
    $data = array();
    $to = (int)$this->input->post('to');
    $lastSample = (int)$this->input->post('lastSample');
    $period = (int)$this->input->post('period');
    $update = ($this->input->post('update') == 'true');
    $data['update'] = $update;
    $data['sampletime'] = array();
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
      $query = $this->db->query('SELECT sampletime / 1000 as sampletime, active_energy, active_power, fundamental_reactive_energy, fundamental_reactive_power FROM agreg_' . $agreg . ' WHERE (sampletime / 1000) > ' . $lastSample . ' AND (sampletime / 1000) >= ' . $from . ' AND (sampletime / 1000) <= ' . $to . ' ORDER BY sampletime ASC');
    } else {
      $query = $this->db->query('SELECT sampletime / 1000 as sampletime, active_energy, active_power, fundamental_reactive_energy, fundamental_reactive_power FROM agreg_' . $agreg . ' WHERE (sampletime / 1000) >= ' . $from . ' AND (sampletime / 1000) <= ' . $to . ' ORDER BY sampletime ASC');      
    }
    foreach($query->result() as $row) {
      $data['sampletime'][] = (int)$row->sampletime;
      $data['active_energy'][] = (float)$row->active_energy;
      $data['active_power'][] = (float)$row->active_power;
      $data['reactive_energy'][] = (float)$row->fundamental_reactive_energy;
      $data['reactive_power'][] = (float)$row->fundamental_reactive_power;
    }
    echo json_encode($data);
  }
  public function ajaxUpdateValues60sEnergyPower() {
    if(!$this->input->is_ajax_request()) return;
    $data = array();
    $Today = strtotime(date('Y-m-d 00:00:00'));      
    $query = $this->db->query('SELECT SUM(active_energy) as energy FROM agreg_10m WHERE (sampletime / 1000) >' . $Today . ' AND ((sampletime / 1000) - 86400) <= ' . $Today)->row();
    $data['today_energy'] = (float)$query->energy;
    echo json_encode($data);
  }
  public function ajaxUpdateValues5sEnergyPower() {
    if(!$this->input->is_ajax_request()) return;
    $data = array();
    $from = time() - 20;
    $row = $this->db->query('SELECT rms_v, rms_i, active_power, freq FROM agreg_3s WHERE (sampletime / 1000) > ' . $from . ' ORDER BY sampletime DESC LIMIT 1')->row();
    $data['rms_v'] = (float)$row->rms_v;
    $data['rms_i'] = (float)$row->rms_i;
    $data['freq'] = (float)$row->freq;
    $data['active_power'] = (float)$row->active_power;
    $this->load->model('Sysinfo_model');
    $data['sysinfo']['ram'] = $this->Sysinfo_model->getServerMemoryUsage();
    $data['sysinfo']['ssd'] = $this->Sysinfo_model->getServerFilesystemUsage();
    $data['sysinfo']['cpu'] = $this->Sysinfo_model->getServerCpuUsage();
    echo json_encode($data);
  }
    
    
    
  
  
  
  
  
  
  
  public function ajaxHarmonicsEvolution() {
    if(!$this->input->is_ajax_request()) return;
    $data = array();
    $start = $this->input->post('start');
    $stop  = $this->input->post('stop');
    //$index = $this->input->post('index');
    $query = $this->db->query('SELECT sampletime / 1000 as sampletime, array_to_json(harmonics_v) AS harmonics_v FROM agreg_1m WHERE sampletime > (SELECT MAX(sampletime) - 86400000 FROM agreg_1m)');
    foreach($query->result() as $row) {
      $data['sampletime'][] = $row->sampletime;
      $data['harmonics_v'][] = json_decode($row->harmonics_v);
    }
    echo json_encode($data);
  }
  public function ajaxHarmonicsEvolution2() {
    if(!$this->input->is_ajax_request()) return;
    $data = array();
    $start = $this->input->post('start');
    $stop  = $this->input->post('stop');
    $index = $this->input->post('index');
    $query = $this->db->query('SELECT harmonics_v[' . $index . '] FROM agreg_1m WHERE (sampletime / 1000 ) > ' . $start . ' AND (sampletime / 1000 ) < ' . $stop);
    foreach($query->result() as $row) {
      $data['harmonics_v'][] = $row->harmonics_v;
    }
    echo json_encode($data);
  }











  public function energy() {
        $time = strtotime('-1 day') * 1000;
        $data['w'] = new stdClass();
        try {
            $query = $this->db->select('sampletime as time, active_energy/1000 as value', false)->where('sampletime >', $time)->order_by('sampletime', 'desc')->get('agreg_10m');
            $res = $query->result();
        } catch (Exception $ex) {
            $res = array();
        }
        $data['w']->one_minute = $res;

        $data['p'] = new stdClass();
        try {
            $query = $this->db->select('sampletime as time, active_power/1000 as value', false)->where('sampletime >', $time)->order_by('sampletime', 'desc')->get('agreg_10m');
            $res = $query->result();
        } catch (Exception $ex) {
            $res = array();
        }
        $data['p']->one_minute = $res;

        $data['d'] = new stdClass();
        try {
            $query = $this->db->select('sampletime as time, fundamental_reactive_energy/1000 as value', false)->where('sampletime >', $time)->order_by('sampletime', 'desc')->get('agreg_10m');
            $res = $query->result();
        } catch (Exception $ex) {
            $res = array();
        }
        $data['d']->one_minute = $res;


        $data['q'] = new stdClass();
        try {
            $query = $this->db->select('sampletime as time, fundamental_reactive_power/1000 as value', false)->where('sampletime >', $time)->order_by('sampletime', 'desc')->get('agreg_10m');
            $res = $query->result();
        } catch (Exception $ex) {
            $res = array();
        }
        $data['q']->one_minute = $res;

        $this->load->model('data_model');
        $data['main_values'] = $this->data_model->getMainValues();


        $this->load->model('event_model');
        $data['numEvents'] = $this->event_model->getNumUnreadEvents($this->session->eventTimestamp);

        $this->load->view('analysis/energy_power_view', $data);
    }

    public function harmonics() {
        $this->load->model('data_model');
        $data['main_values'] = $this->data_model->getMainValues();

        $this->load->model('event_model');
        $data['numEvents'] = $this->event_model->getNumUnreadEvents($this->session->eventTimestamp);

        $this->load->view('analysis/harmonics_view', $data);
    }

    public function frequency() {
        $this->load->model('event_model');

        $data['frequency'] = new stdClass();
        $timestamp = strtotime('00:00:00') * 1000;
        try {
            $query = $this->db->select('sampletime as time, freq as value')->where('sampletime >', strtotime('00:00:00') * 1000)->order_by('sampletime', 'desc')->get('agreg_10m');
            $res = $query->result();
        } catch (Exception $ex) {
            $res = array();
        }
        $data['frequency']->one_minute = $res;
        try {
            $query = $this->db->select('avg(freq) as mean, max(freq) as max, min(freq) as min')->where('sampletime >', strtotime('00:00:00') * 1000)->get('agreg_10m');
            $res = $query->row();
        } catch (Exception $ex) {
            $res = array();
        }
        if (count($res) > 0) {
            $data['frequency']->today = $res->mean;
            $data['frequency']->max_today = $res->max;
            $data['frequency']->min_today = $res->min;
        } else {
            $data['frequency']->today = 0;
            $data['frequency']->max_today = 0;
            $data['frequency']->min_today = 0;
        }

        $this->load->model('event_model');
        $data['events'] = array(); //$this->event_model->getEvents(strtotime('today') * 1000, strtotime('today + 1 day') * 1000);
        //print_r($data['events']);
        $this->load->model('data_model');
        $data['main_values'] = $this->data_model->getMainValues();

        $data['numEvents'] = $this->event_model->getNumUnreadEvents($this->session->eventTimestamp);

        $this->load->view('analysis/frequency_view', $data);
    }




    public function ajaxGetVoltageWaveform() {
        if ($this->input->is_ajax_request()) {
            $snd_msg = "OSCILLOSCOPE|" . $this->input->post('lastTimestamp');
            $rcv_msg = "";
            $timeout = 5;
            $socket = stream_socket_client('unix:///var/run/openZmeter.sock', $errorno, $errorstr, $timeout);
            stream_set_timeout($socket, $timeout);

//echo("Sending Message...<br>\n");
            if (!fwrite($socket, $snd_msg)) {
                echo "-1";
                return;
            }
//echo("Error while writing!!!<br>\n");
//echo("Receiving Message...<br>\n");
            do {
                $data = @fread($socket, 8192);
                if (strlen($data) == 0) {
                    break;
                }
                $rcv_msg .= $data;
            } while (true);
//$points = json_decode($rcv_msg);
//print_r($points);
            echo $rcv_msg;
        }
    }

    public function ajaxGetHarmonics() {
        if ($this->input->is_ajax_request()) {

            $this->load->model('data_model');
            $result = $this->data_model->getHarmonicsInfo();
//            $this->influxdb->setTimePrecision('ms');
//            $db = $this->influxdb->getDatabase();
//            switch ($this->input->post('metric')) {
//                case 'voltage':
//                    $serie = 'harm_voltage';
//                    break;
//                case 'current':
//                    $serie = 'harm_current';
//                    break;
//                default:
//                    $serie = 'harm_voltage, harm_current, power_pf, power_thd_u, power_thd_i';
//                    break;
//            }
//            try {
//                $res = $db->query("SELECT * FROM {$serie} " /* WHERE time > now() - 3s */ . "order by time desc  LIMIT 1;");
//                if (!isset($res['harm_voltage'])) {
//                    $harmonics = -1;
//                } else {
//                    if ($this->input->post('metric') != 'all') {
//                        $harmonics = $res[$serie];
//                    } else {
//                        $harmonics = $res;
//                    }
//                }
//            } catch (Exception $e) {
//                $harmonics = -1;
//            }

            echo json_encode($result);
        }
    }

    public function ajaxGetVoltageHarmonics() {
        if ($this->input->is_ajax_request()) {
            $this->load->model('data_model');
            $harmonics = $this->data_model->getHarmonics('harmonics_v');
//
//            //$this->influxdb->setTimePrecision('ms');
//            //$db = $this->influxdb->getDatabase();
//            try {
//                $res = $db->query("SELECT * FROM harm_voltage " /* WHERE time > now() - 3s */ . "order by time desc LIMIT 1;");
//                if (!isset($res['harm_voltage'])) {
//                    $harmonics = -1;
//                } else {
//                    $harmonics = $res['harm_voltage'];
//                }
//            } catch (Exception $e) {
//                $harmonics = -1;
//            }

            echo json_encode($harmonics);
        }
    }

    public function ajaxGetVoltageFFT() {
        if ($this->input->is_ajax_request()) {
            $snd_msg = "FFT_V|" . $this->input->post('lastTimestamp');
            $rcv_msg = "";
            $timeout = 5;
            $socket = stream_socket_client('unix:///var/run/openZmeter.sock', $errorno, $errorstr, $timeout);
            stream_set_timeout($socket, $timeout);

//echo("Sending Message...<br>\n");
            if (!fwrite($socket, $snd_msg)) {
                echo "-1";
            }
//echo("Error while writing!!!<br>\n");
//echo("Receiving Message...<br>\n");
            do {
                $data = @fread($socket, 8192);
                if (strlen($data) == 0) {
                    break;
                }
                $rcv_msg .= $data;
            } while (true);
//$points = json_decode($rcv_msg);
//print_r($points);
            echo $rcv_msg;
        }
    }

    public function ajaxGetCurrentFFT() {
        if ($this->input->is_ajax_request()) {
            $snd_msg = "FFT_I|" . $this->input->post('lastTimestamp');
            $rcv_msg = "";
            $timeout = 5;
            $socket = stream_socket_client('unix:///var/run/openZmeter.sock', $errorno, $errorstr, $timeout);
            stream_set_timeout($socket, $timeout);

//echo("Sending Message...<br>\n");
            if (!fwrite($socket, $snd_msg)) {
                echo "-1";
            }
//echo("Error while writing!!!<br>\n");
//echo("Receiving Message...<br>\n");
            do {
                $data = @fread($socket, 8192);
                if (strlen($data) == 0) {
                    break;
                }
                $rcv_msg .= $data;
            } while (true);
//$points = json_decode($rcv_msg);
//print_r($points);
            echo $rcv_msg;
        }
    }

    public function ajaxGetCurrentWaveform() {
        if ($this->input->is_ajax_request()) {
            $snd_msg = "OSCILLOSCOPE|" . $this->input->post('lastTimestamp');
            $rcv_msg = "";
            $timeout = 5;
            $socket = stream_socket_client('unix:///var/run/openZmeter.sock', $errorno, $errorstr, $timeout);
            stream_set_timeout($socket, $timeout);

//echo("Sending Message...<br>\n");
            if (!fwrite($socket, $snd_msg)) {
                echo "-1";
            }
//echo("Error while writing!!!<br>\n");
//echo("Receiving Message...<br>\n");
            do {
                $data = @fread($socket, 8192);
                if (strlen($data) == 0) {
                    break;
                }
                $rcv_msg .= $data;
            } while (true);
//$points = json_decode($rcv_msg);
//print_r($points);
            echo $rcv_msg;
        }
    }

    public function ajaxGetCurrentHarmonics() {
        if ($this->input->is_ajax_request()) {
            $this->load->model('data_model');
            $harmonics = $this->data_model->getHarmonics('harmonics_i');
//
//            //$this->influxdb->setTimePrecision('ms');
//            //$db = $this->influxdb->getDatabase();
//            try {
//                $res = $db->query("SELECT * FROM harm_voltage " /* WHERE time > now() - 3s */ . "order by time desc LIMIT 1;");
//                if (!isset($res['harm_voltage'])) {
//                    $harmonics = -1;
//                } else {
//                    $harmonics = $res['harm_voltage'];
//                }
//            } catch (Exception $e) {
//                $harmonics = -1;
//            }

            echo json_encode($harmonics);
        }
    }

    public function ajaxGetRangedValues() {
        if ($this->input->is_ajax_request()) {
            $this->load->model('data_model');
            $res = $this->data_model->getRangedValues($this->input->post('metric'), $this->input->post('startDate'), $this->input->post('endDate'));
            echo json_encode($res);
        }
    }

    public function ajaxGetSamples() {
        if ($this->input->is_ajax_request()) {
            $this->load->model('data_model');
            $res = $this->data_model->getSamples($this->input->post('metric'), $this->input->post('startDate'), $this->input->post('endDate'));
            echo json_encode($res);
        }
    }

    public function ajaxGetEvents() {
        if ($this->input->is_ajax_request()) {
            $this->load->model('event_model');
            $res = $this->event_model->getEvents($this->input->post('startDate'), $this->input->post('endDate'), NULL, $this->input->post('getRVC'));
            echo json_encode($res);
        }
    }

}
