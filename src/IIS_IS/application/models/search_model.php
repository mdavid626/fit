<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');


/**
 * Description of classroom_model
 * Model implementing database querys for searching.
 * @author xmolna02
 */
class Search_model extends CI_Model {
    
    public function search_classroom()
    {
        $this->load->helper('date');

        $this->db->distinct();
        $this->db->select('Ucebna.id_miest, typ_miest, kapacita, spec_vyb');

        $this->db->join('Rezervacia', 'Ucebna.id_miest = Rezervacia.id_miest', 'left outer');
        $this->db->join('Vyuka', 'Ucebna.id_miest = Vyuka.id_miest', 'left outer');

        if ($this->input->post('by_room_id'))
        {
            $this->db->where('Ucebna.id_miest', $this->input->post('room_id'));
        }

        if ($this->input->post('by_miesto'))
        {
            $this->db->where('typ_miest', $this->input->post('miesto'));
        }

        if ($this->input->post('by_kapacita'))
        {
            $this->db->where('kapacita >=', $this->input->post('kapacita'));
        }

        if ($this->input->post('by_vybavenie'))
        {
            $this->db->like('spec_vyb', $this->input->post('vybavenie'));
        }

        if ($this->input->post('by_datum1'))
        {
            /*$date = $this->global_model->sk_to_date($this->input->post('datum1'));
            $this->db->where("(ISNULL(Rezervacia.cas_zac) OR ISNULL(Rezervacia.cas_ukon) OR Rezervacia.cas_zac > '$date' OR Rezervacia.cas_ukon <= '$date') AND (ISNULL(Vyuka.cas_zac) OR ISNULL(Vyuka.cas_ukon) OR Vyuka.cas_zac > '$date' OR Vyuka.cas_ukon <= '$date')", NULL, FALSE);*/

            $date = sk_to_date($this->input->post('datum1'));

            $this->db->where('(ISNULL(`Rezervacia`.`cas_zac`)', NULL, FALSE);
            $this->db->or_where('Rezervacia.cas_ukon', NULL);
            $this->db->or_where('Rezervacia.cas_zac >', $date);
            $this->db->or_where("`Rezervacia`.`cas_ukon` <= " . $this->db->escape($date) . ")", NULL, FALSE);

            $this->db->where('(ISNULL(`Vyuka`.`cas_zac`)', NULL, FALSE);
            $this->db->or_where('Vyuka.cas_ukon', NULL);
            $this->db->or_where('Vyuka.cas_zac >', $date);
            $this->db->or_where("`Vyuka`.`cas_ukon` <= " . $this->db->escape($date) . ")", NULL, FALSE);
        }

        if ($this->input->post('by_datum2'))
        {
            $date = sk_to_date($this->input->post('datum2'));

            $this->db->where('(ISNULL(`Rezervacia`.`cas_zac`)', NULL, FALSE);
            $this->db->or_where('Rezervacia.cas_ukon', NULL);
            $this->db->or_where('Rezervacia.cas_zac >=', $date);
            $this->db->or_where("`Rezervacia`.`cas_ukon` < " . $this->db->escape($date) . ")", NULL, FALSE);

            $this->db->where('(ISNULL(`Vyuka`.`cas_zac`)', NULL, FALSE);
            $this->db->or_where('Vyuka.cas_ukon', NULL);
            $this->db->or_where('Vyuka.cas_zac >=', $date);
            $this->db->or_where("`Vyuka`.`cas_ukon` < " . $this->db->escape($date) . ")", NULL, FALSE);
        }

        $query = $this->db->get('Ucebna');

        $result = $query->result();

        return $result;
    }
    
    public function search_reservation()
    {
        $this->load->helper('date');

        $this->db->select('DISTINCT `id_miest`, `typ_udal`, `zobraz_meno`, `cas_zac`, `cas_ukon`', FALSE);

        if ($this->input->post('by_room_id'))
        {
            $this->db->where('id_miest', $this->input->post('room_id'));
        }

        if ($this->input->post('by_datum1'))
        {
            $this->db->where('cas_zac >=', sk_to_date($this->input->post('datum1')));
        }

        if ($this->input->post('by_datum2'))
        {
            $this->db->where('cas_ukon <=', sk_to_date($this->input->post('datum2')));
        }

        $this->db->join('Zamestnanec', 'Zamestnanec.r_cislo = Rezervacia.r_cislo');

        $query = $this->db->get('Rezervacia');

        $result = $query->result();

        foreach ($result as $row)
        {
            $row->cas_zac = format_date($row->cas_zac);
            $row->cas_ukon = format_date($row->cas_ukon);
        }

        return $result;
    }
    
    public function search_schedule()
    {   
        //$this->load->model('global_model');
       
        if ($this->input->post('by_ucebna'))
        {
            $this->db->where('id_miest', $this->input->post('ucebna'));
        }
        
        if ($this->input->post('by_obor'))
        {
            $this->db->where('stud_obor', $this->input->post('obor'));
        }
        
        if ($this->input->post('by_rocnik'))
        {
            $this->db->where('rocnik', $this->input->post('rocnik'));
        }
        
        if ($this->input->post('by_datum1'))
        {
            $this->db->where('cas_zac >=', sk_to_date($this->input->post('datum1')));
        }
        
        if ($this->input->post('by_datum2'))
        {
            $this->db->where('cas_ukon <=', sk_to_date($this->input->post('datum2')));
        }
        
        $this->db->join('Zamestnanec', 'Zamestnanec.r_cislo = Vyuka.r_cislo');
        
        $query = $this->db->get('Vyuka');
        
        $result = $query->result();
        
        foreach ($result as $row)
        {
            $row->cas_zac = format_date($row->cas_zac);
            $row->cas_ukon = format_date($row->cas_ukon);
        }
        
        return $result;
    }
    
    public function get_typ_miesta()
    {
        //SELECT DISTINCT typ_miest FROM `Ucebna`
        $this->db->select('DISTINCT `typ_miest`', FALSE);
        return $this->db->get('Ucebna');
    }
    
    public function get_ucebne()
    {
        //SELECT DISTINCT id_miest FROM `Ucebna`
        $this->db->select('DISTINCT `id_miest`', FALSE);
        return $this->db->get('Ucebna');
    }
    
    public function get_obor()
    {
        //SELECT DISTINCT stud_obor FROM `Vyuka`
        $this->db->select('DISTINCT `stud_obor`', FALSE);
        return $this->db->get('Vyuka');
    }
    
    public function get_rocnik()
    {
        //SELECT DISTINCT rocnik FROM `Vyuka`
        $this->db->select('DISTINCT `rocnik`', FALSE);
        return $this->db->get('Vyuka');
    }
    
}

/* End of file search_model.php */
/* Location: ./application/models/search_model.php */