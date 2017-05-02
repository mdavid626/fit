<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Description of classroom_model
 * Model implementing database operations with users.
 * @author xmesza03, xmolna02
 */
class User_model extends CI_Model {
    
    public function add_reservation()
    {
        $this->load->helper('date');
	   
        $this->db->set('r_cislo', $this->session->userdata('r_cislo'));
        $this->db->set('id_miest', $this->input->post('ucebna'));
        $this->db->set('typ_udal', $this->input->post('typ'));
        $this->db->set('cas_zac', sk_to_date($this->input->post('datum1')));
        $this->db->set('cas_ukon', sk_to_date($this->input->post('datum2')));
        
        $query = $this->db->insert('Rezervacia');
        
        return $query;
    }
    
    public function check_free_reservation()
    {
        $this->load->helper('date');
        
        $this->db->distinct();
        $this->db->select('Ucebna.id_miest, typ_miest, kapacita, spec_vyb');

        $this->db->join('Rezervacia', 'Ucebna.id_miest = Rezervacia.id_miest', 'left outer');
        $this->db->join('Vyuka', 'Ucebna.id_miest = Vyuka.id_miest', 'left outer');
        
        $this->db->where('Ucebna.id_miest', $this->input->post('ucebna'));
        
        $date = sk_to_date($this->input->post('datum1'));

        $this->db->where('(ISNULL(`Rezervacia`.`cas_zac`)', NULL, FALSE);
        $this->db->or_where('Rezervacia.cas_ukon', NULL);
        $this->db->or_where('Rezervacia.cas_zac >', $date);
        $this->db->or_where("`Rezervacia`.`cas_ukon` <= " . $this->db->escape($date) . ")", NULL, FALSE);

        $this->db->where('(ISNULL(`Vyuka`.`cas_zac`)', NULL, FALSE);
        $this->db->or_where('Vyuka.cas_ukon', NULL);
        $this->db->or_where('Vyuka.cas_zac >', $date);
        $this->db->or_where("`Vyuka`.`cas_ukon` <= " . $this->db->escape($date) . ")", NULL, FALSE);

        $date = sk_to_date($this->input->post('datum2'));

        $this->db->where('(ISNULL(`Rezervacia`.`cas_zac`)', NULL, FALSE);
        $this->db->or_where('Rezervacia.cas_ukon', NULL);
        $this->db->or_where('Rezervacia.cas_zac >=', $date);
        $this->db->or_where("`Rezervacia`.`cas_ukon` < " . $this->db->escape($date) . ")", NULL, FALSE);

        $this->db->where('(ISNULL(`Vyuka`.`cas_zac`)', NULL, FALSE);
        $this->db->or_where('Vyuka.cas_ukon', NULL);
        $this->db->or_where('Vyuka.cas_zac >=', $date);
        $this->db->or_where("`Vyuka`.`cas_ukon` < " . $this->db->escape($date) . ")", NULL, FALSE);
        
        /*$this->db->where('id_miest', $this->input->post('ucebna'));
        $this->db->where('cas_zac >=', sk_to_date($this->input->post('datum1')));
        $this->db->where('cas_ukon <=', sk_to_date($this->input->post('datum2')));
        
        $query = $this->db->get('Rezervacia');*/
        $query = $this->db->get('Ucebna');
        
        return $query->num_rows();
    }
    
    public function check_zam_free()
    {
        
        $this->db->where('r_cislo', $this->input->post('r_cislo'));
        $query = $this->db->get('Zamestnanec');
        
        return !$query->num_rows();
    }
    
    function validate()
    {
        $this->db->where('username', $this->input->post('username'));
        $this->db->where('heslo', md5($this->input->post('password')));
        $query = $this->db->get('Zamestnanec');

        $result = array();
        $result['role'] = M_UNAUTH;
        $result['username'] = '';
        $result['fname'] = '';
        $result['r_cislo'] = '';

        if($query->num_rows == 1)
        {
            $result['username'] = $query->row()->username;
            $result['role'] = $query->row()->role;
            $result['fname'] = $query->row()->zobraz_meno;
            $result['r_cislo'] = $query->row()->r_cislo;
        }

        return $result;
    }
    
    public function get_users()
    {
        $this->db->where('role >', 1);
        $query = $this->db->get('Zamestnanec');
        return $query->result();
    }
    
    public function get_users_by($key, $value)
    {
        $this->db->where($key, $value);
        $query = $this->db->get('Zamestnanec');
        return $query->result();
    }
    
    public function add_user()
    {
        
        $this->db->set('r_cislo', $this->input->post('r_cislo'));
        $this->db->set('role', M_USER);
        $this->db->set('username', $this->input->post('username'));
        $this->db->set('heslo', md5($this->input->post('heslo')));
        $this->db->set('meno', $this->input->post('meno'));
        $this->db->set('priezvisko', $this->input->post('priezvisko'));
        $this->db->set('zobraz_meno', $this->input->post('zobraz_meno'));
        $this->db->set('titul', $this->input->post('titul'));
        $this->db->set('prac_pomer', $this->input->post('prac_pomer'));
        
        
        
        $query = $this->db->insert('Zamestnanec');
        
        return $query;
        
    }
    
    public function remove_user($userid)
    {
        $this->db->where("r_cislo", $userid);
        $this->db->delete('Zamestnanec');
    }
    
    public function edit_user($data)
    {
        $this->db->where('r_cislo', $data['r_cislo']);
        unset($data['r_cislo']);
        $this->db->update('Zamestnanec', $data);
    }
}

/* End of file user_model.php */
/* Location: ./application/models/user_model.php */