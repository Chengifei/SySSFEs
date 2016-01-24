import alg

class geom():
	
	def plot(self):
		pass

class polygon(geom):
	
	def __init__(self, *args):
		self.vertex=args
		self.n=len(self.vertex)
		
	def __sort(self):
		C=alg.vector(sum(A[0]/self.n for A in self.vertex),
				   sum(A[1]/self.n for A in self.vertex))
		det=[]
		for i in self.vertex:
			t=i-C
			det=[t.cross(i) for i in self.vertex]
		for i in det:
			pass
	
	def __matrize(self):
		pass
	
	def __chebCenter(self):
		pass
		
class circle(geom):
	
	def __init__(self, center = None, radius = None):
		self.center=center
		self.radius=radius
		
class funcCurve(geom):
	
	def __init__(self, x = None, y = None):
		pass