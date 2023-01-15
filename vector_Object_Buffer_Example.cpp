



struct vertex3D
{
    float  x, y, z;
};





vector<vertex3D> world_Vertex_Buffer;



//write objects to vector
for (int i = 0; i < 10; i++)
{
    vertex3D vertex_n;//object template

    vertex_n.x = i;  //modify object data
    vertex_n.y = 2;
    if (i == 500) vertex_n.z = 55;
    else vertex_n.z = 0;

    world_Vertex_Buffer.push_back(vertex_n);//push object into vector buffer   
}


//read objects from vector
for (int i = 0; i < world_Vertex_Buffer.size(); i++)
{
    vertex3D  temp = world_Vertex_Buffer[i];//object template copied from vector buffer
    float X = temp.x;
    float Y = temp.y;
    float Z = temp.z;

    Serial.println(X);//use data
    Serial.println(Y);
    Serial.println(Z);
    Serial.println();
}