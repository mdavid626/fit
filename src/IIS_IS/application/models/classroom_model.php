<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Description of classroom_model
 * Model implementing database operations with classrooms.
 * @author xmesza03
 */
class classroom_model extends CI_Model {
    
    public function remove_classroom($id)
    {
        $this->db->where("id_miest", $id);
        $this->db->delete('Ucebna');
    }
    
    public function get_classroom($id)
    {
        $this->db->where('Ucebna.id_miest', $id);
        $query = $this->db->get('Ucebna');
        $result = $query->result();
        return $result;
    }
    
    public function add_classroom($data)
    {
        $result = $this->get_classroom($data['id_miest']);
        
        if( empty($result) ) 
        {
            $this->db->insert('Ucebna', $data);
            return TRUE;
        } else 
        {
            return FALSE;
        }
        
    }
    
    /**
     * Editing data about a classroom
     * @param array $data Contains classroom's data
     * @param String $original Name of the original classroom intended for editation.
     * In case of changing the name of the classroom (primary key) it has to be
     * added as a new row to the database and also delete the old record, whose
     * name is contained in this variable.
     */
    public function edit_classroom($data, $original)
    {
        $this->db->where('id_miest', $data['id_miest']);
        $query = $this->db->get('Ucebna');
        $result = $query->result();
        if( empty( $result ) )
        {
            $this->remove_classroom($original);
            $this->db->insert('Ucebna', $data);
        } else
        {
            unset($data['id_miest']);
            $this->db->update('Ucebna', $data);
        }
        
        
    }
    
}

?>
