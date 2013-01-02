import designspacetoolbox_test
import numpy as np
import matplotlib
import mpl_toolkits.mplot3d.axes3d as Axes3D
import mpl_toolkits.mplot3d.art3d as art3d
import math
import itertools
from collections import OrderedDict

class VariablePool(dict):
        """ A python class of the DSVariablePool object"""
        _data = None
        def __init__(self, *args, **kwargs):
                self.update(*args, **kwargs)
        def __del__(self):
                if self._data == None:
                        return
                designspacetoolbox_test.DSVariablePoolSetReadWriteAdd(self._data)
                designspacetoolbox_test.DSVariablePoolFree(self._data)
        def __setattr__(self, name, value):
                if name == '_data':
                        if self._data == None:
                                for i in xrange(0, designspacetoolbox_test.DSVariablePoolNumberOfVariables(value)):
                                        variable = designspacetoolbox_test.DSVariablePoolVariableAtIndex(value, i)
                                        self[variable[0]] = variable[1]
                        else:
                                raise AttributeError, 'The _data field is immutable once assigned'
                return
        def __setitem__(self, name, value):
                if self._data == None:
                        self.__dict__['_data'] = designspacetoolbox_test.DSVariablePoolAlloc()
                if self.has_key(name) == False:
                        self._addVariable(name)
                self._setValueForVariable(name, value)
                super(VariablePool, self).__setitem__(name, value)
        def __getitem__(self, name):
                val = super(VariablePool, self).__getitem__(name)
                return val
        def update(self, *args, **kwargs):
                for k, v in dict(*args, **kwargs).iteritems():
                        self[k] = v
        def _setValueForVariable(self, name, value):
                name=str(name)
                value=float(value)
                designspacetoolbox_test.DSVariablePoolSetValueForVariableWithName(self._data,name,value)
                return
        def _addVariable(self, name):
                if type(name)==str:
                        designspacetoolbox_test.DSVariablePoolAddVariableWithName(self._data, name)
        def __repr__(self):
                if self._data == None:
                        return ''
                numberOfVariables = len(self)
                repr=str()
                variableIndices = range(numberOfVariables)
                variableIndices.reverse()
                for i in variableIndices:
                        variable = self.variableAtIndex(i)
                        repr += variable[0] + ':\t' + str(variable[1]) + '\n'
                return repr
        def _hasVariable(self, name):
                if type(name)!= str:
                        return False
                return designspacetoolbox_test.DSVariablePoolHasVariableWithName(self._data, name)
        def copy(self):
                if self._data == None:
                        return None
                newPool = VariablePool()
                newPool._data = self._data
                return newPool
        def indexOfVariableWithName(self, name):
                if self._data == None:
                        return None
                return designspacetoolbox_test.DSVariablePoolIndexOfVariableWithName(self._data, name)
        def variableAtIndex(self, index):
                if index >= len(self):
                        return None
                return designspacetoolbox_test.DSVariablePoolVariableAtIndex(self._data, index)


class DesignSpacePlot:
        Variables = (None, None)
        Limits = ((None, None), (None, None))
        Colormap = OrderedDict()
        Mode = 'Slice'
        Dspace = None
        Xi=None
        ax=[None,None]
        Zlim=None
        def __init__(self, dspace, point, xvariable, xlim, yvariable, ylim, Mode='Slice', Colormap=None):
                Variables = dict()
                self.__dict__['Dspace'] = dspace
                self.__dict__['Xi'] = point                
                self.__dict__['Variables'] = {'X' : xvariable, 'Y' : yvariable}
                self.__dict__['Limits'] = {'X' : (min(xlim), max(xlim)),
                                           'Y' : (min(ylim), max(ylim))}
                if Colormap != None:
                        self.__dict__['Colormap'] = Colormap
                self.__dict__['Mode'] = Mode
                Xi = dspace.independentVariables
                if Xi.has_key(self.Variables['X']) == False:
                        self = None
                        raise ValueError, 'Design Space Plot Axis is not an Independnet Variable:' + self.Variables['X']
                if Xi.has_key(self.Variables['Y']) == False:
                        self = None
                        raise ValueError, 'Design Space Plot Axis is not an Independnet Variable:' + self.Variables['Y']
        def draw(self, algebraic=None, colorbar=True, function=None, resolution=100, intersections=range(2, 100), isLogLinear=False, contourf=False, levels=None, boundaries=False):
                if ((str(self.Mode).lower() == 'function') & (function == None)):
                        raise ValueError, 'Draw Function mode requires a function for z direction'
                lower = self.Xi.copy()
                upper = self.Xi.copy()
                lower[self.Variables['X']] = min(self.Limits['X'])
                lower[self.Variables['Y']] = min(self.Limits['Y'])
                upper[self.Variables['X']] = max(self.Limits['X'])
                upper[self.Variables['Y']] = max(self.Limits['Y'])
                if str(self.Mode).lower() == 'slice':
                        self._plot2DSlice(lower, upper, colorbar, intersections)
                if str(self.Mode).lower() == 'function':
                        self._plot2DFunction(lower, upper, colorbar, function, resolution, isLogLinear, contourf=contourf)
                if str(self.Mode).lower() == 'routh':
                        self._plot2DRouth(algebraic, lower, upper, colorbar, resolution, levels=levels, contourf=contourf)
                if boundaries==True and str(self.Mode).lower() != 'slice':
                        self._plot2DBoundaries(lower, upper)
                matplotlib.pyplot.ylim([math.log10(i) for i in self.Limits['Y']])
                matplotlib.pyplot.xlim([math.log10(i) for i in self.Limits['X']])
                matplotlib.pyplot.xlabel(r'$\log_{10}('+str(self.Variables['X'])+')$')
                matplotlib.pyplot.ylabel(r'$\log_{10}('+str(self.Variables['Y'])+')$')
        def draw_case(self, case_number, function=None, resolution=100, isLogLinear=False, contourf=False):
                if ((str(self.Mode).lower() == 'function') & (function == None)):
                        raise ValueError, 'Draw Function mode requires a function for z direction'
                lower = self.Xi.copy()
                upper = self.Xi.copy()
                lower[self.Variables['X']] = min(self.Limits['X'])
                lower[self.Variables['Y']] = min(self.Limits['Y'])
                upper[self.Variables['X']] = max(self.Limits['X'])
                upper[self.Variables['Y']] = max(self.Limits['Y'])
                cases = self.Dspace.validCases(lower=lower, upper=upper)
                keys = [str(case.caseNumber) for case in cases]
                self._addToColormap(keys)
                case = self.Dspace.caseWithCaseNumber(case_number)
                if str(self.Mode).lower() == 'slice':
                        self._plot_case2DSlice(case, lower, upper)
                if str(self.Mode).lower() == 'function':
                        expr = designspacetoolbox_test.DSExpressionByParsingString(function)
                        self._plot_case2DFunction(case, lower, upper, expr, resolution, isLogLinear, contourf=contourf)
                        designspacetoolbox_test.DSExpressionFree(expr)
                matplotlib.pyplot.ylim([math.log10(i) for i in self.Limits['Y']])
                matplotlib.pyplot.xlim([math.log10(i) for i in self.Limits['X']])
                matplotlib.pyplot.xlabel(r'$\log_{10}('+str(self.Variables['X'])+')$')
                matplotlib.pyplot.ylabel(r'$\log_{10}('+str(self.Variables['Y'])+')$')
                
        def _addToColormap(self, keys):
                if self.Colormap == None:
                        self.Colormap = OrderedDict()
                for key in keys:
                        if self.Colormap.has_key(key) == False:
                                self.Colormap[key] = 1
                cm=matplotlib.colors.Normalize()
                temp = len(self.Colormap.keys())
                cm.autoscale(range(temp+1))
                for i in xrange(0, len(self.Colormap.keys())):
                    color = matplotlib.colors.rgb2hex(matplotlib.cm.hsv(cm(i)))
                    self.Colormap[self.Colormap.keys()[i]] = color

        def _regionColorbar(self, ax, keys):
                keys.reverse()
                ax.set_aspect(20./len(keys), 'box')
                V = np.array([(0,0), (1,0), (1,1), (0,1)])
                for i in xrange(0, len(keys)):
                        matplotlib.pyplot.fill(V[:,0], V[:,1]+i, self.Colormap[keys[i]], hold=True)
                ax.set_xlim([0, 1])
                ax.set_xticks([])
                ax.set_ylim([0, len(keys)])
                ax.yaxis.set_ticks_position('right')
                ax.set_yticks(np.array(range(0,len(keys)), dtype='float')+0.5)
                ax.set_yticklabels(keys)
                ax.set_title('Cases')
        def _functionColorbar(self, ax, zlim):
                x=np.array([0., 1.])
                y0 = zlim[0]
                y1 = zlim[1]
                ax.set_aspect(20/(y1-y0))
                y=np.linspace(y0, y1, 250)
                X, Y = np.meshgrid(x, y)
                Z = matplotlib.mlab.griddata([0, 1, 1, 0], [y0, y0, y1, y1], [y0, y0, y1, y1], x, y)
                ax.pcolor(X, Y, Z, cmap=matplotlib.cm.jet, vmin=y0, vmax=y1)
                ax.set_xlim([0, 1])
                ax.set_xticks([])
                ax.set_yticks(np.round(np.linspace(y0, y1, 8), 3))
                ax.set_ylim(np.round([y0, y1], 3))
                ax.yaxis.set_ticks_position('right')
                ax.set_title('Function')
        def _plot_case2DSlice(self, case, lower, upper):
                V = case.verticesFor2DSlice(self.Xi,
                                            self.Variables['X'],
                                            self.Variables['Y'],
                                            self.Limits['X'],
                                            self.Limits['Y'])
                if V != []:
                        color = self.Colormap[str(case.caseNumber)]
                        matplotlib.pyplot.fill(V[:,0], V[:,1], color, hold=True)
        def _plot2DBoundaries(self, lower, upper):
                cases = self.Dspace.validCases(lower=lower, upper=upper)
                keys = [str(case.caseNumber) for case in cases]
                for case in cases:
                    V = case.verticesFor2DSlice(self.Xi,
                                                self.Variables['X'],
                                                self.Variables['Y'],
                                                self.Limits['X'],
                                                self.Limits['Y'])
                    if V != []:
                        matplotlib.pyplot.fill(V[:,0], V[:,1], fc='none', ec='k', hold=True)
        def _plot2DSlice(self, lower, upper, colorbar, intersections, fill=True, boundaries=True):
                cases = self.Dspace.validCases(lower=lower, upper=upper)
                Ints =self.Dspace.findIntersections(cases, n=intersections, lower=lower, upper=upper)
                keys = [str(case.caseNumber) for case in cases]
                if Ints != None:
                        for intersectGroup in Ints:
                                for intersection in intersectGroup:
                                        key=','.join([str(case.caseNumber) for case in intersection])
                                        keys.append(key)
                self._addToColormap(keys)
                for case in cases:
                        self._plot_case2DSlice(case, lower, upper)
                if Ints != None:
                        for intersectGroup in Ints:
                                for intersection in intersectGroup:
                                        V = np.asarray(
                                                designspacetoolbox_test.DSCaseIntersectionVerticesForSlice(len(intersection),
                                                                                                           [case._data for case in intersection],
                                                                                                           lower._data, 
                                                                                                           upper._data,
                                                                                                           2,
                                                                                                           [self.Variables['X'], self.Variables['Y']]))
                                        if V != []:
                                                key=','.join([str(case.caseNumber) for case in intersection])
                                                color = self.Colormap[key]                                               
                                                matplotlib.pyplot.fill(V[:,0], V[:,1], fc=color, hold=True)
                if colorbar == True:
                        axis = matplotlib.pyplot.gca()
                        ax, _ = matplotlib.colorbar.make_axes(axis)
                        self._regionColorbar(ax, keys)
                        matplotlib.pyplot.sca(axis)
        def _generateMeshForRegion(self, V, resolution):
                xPoints = None
                yPoints = None
                for i in xrange(len(V)):
                        x1=V[i, 0]
                        y1=V[i,1]
                        x2=V[(i+1)%len(V), 0]
                        y2=V[(i+1)%len(V), 1]
                        xNumPoint = (x2-x1)*resolution/(math.log10(max(self.Limits['X']))-math.log10(min(self.Limits['X'])))
                        yNumPoint = (y2-y1)*resolution/(math.log10(max(self.Limits['Y']))-math.log10(min(self.Limits['Y'])))
                        xNumPoint = np.ceil(abs(xNumPoint))
                        yNumPoint = np.ceil(abs(yNumPoint))
                        if x1 == x2:
                                y = np.linspace(y1, y2, yNumPoint)
                                x = np.array(x1)
                        elif y1 == y2:
                                y = np.array(y1)
                                x = np.linspace(x1, x2, xNumPoint)
                        else:
                                m = (y2-y1)/(x2-x1)
                                C = y1-m*x1
                                x = np.linspace(x1, x2, xNumPoint)
                                y = m*x+C
                        if xPoints == None:
                                xPoints = x
                                yPoints = y
                        else:
                                xPoints = np.append(xPoints, x)
                                yPoints = np.append(yPoints, y)
                xPoints = np.unique(xPoints)
                yPoints = np.unique(yPoints)
                return (xPoints, yPoints)
        def _plot_case2DFunction(self, case, lower, upper, expr, resolution, isLogLinear=False, contourf=False):
                Var = self.Xi.copy()
                designspacetoolbox_test.DSVariablePoolSetReadWriteAdd(Var._data)
                contours=list()
                ax=matplotlib.pyplot.gca()
                zlim = self.Zlim
                V=case.verticesFor2DSlice(self.Xi,
                                          self.Variables['X'],
                                          self.Variables['Y'],
                                          self.Limits['X'],
                                          self.Limits['Y'])
                if V != []:
                    Vx=np.sort(V[:,0])
                    Vy=np.sort(V[:,1])
                    xNumPoint = (max(Vx)-min(Vx)+2)*resolution/(math.log10(max(self.Limits['X']))-math.log10(min(self.Limits['X'])));
                    yNumPoint = (max(Vy)-min(Vy)+2)*resolution/(math.log10(max(self.Limits['Y']))-math.log10(min(self.Limits['Y'])));
                    maxNumPoints = max([xNumPoint, yNumPoint])
                    x, y = self._generateMeshForRegion(V, resolution)
                    X,Y = np.meshgrid(x, y)
                    if isLogLinear == True:
                        F = list()
                        for i in xrange(len(V[:,0])):
                            self.Xi[self.Variables['X']] = 10**V[i,0]
                            self.Xi[self.Variables['Y']] = 10**V[i,1]
                            Var[self.Variables['X']] = 10**V[i,0]
                            Var[self.Variables['Y']] = 10**V[i,1]
                            SS = case.steadyStateAtPoint(self.Xi)
                            flux = case.steadyStateFluxAtPoint(self.Xi)
                            for k in xrange(len(SS)):
                                Var[self.Dspace.dependentVariables.variableAtIndex(k)[0]] = 10**SS[k]
                                Var['V_' + self.Dspace.dependentVariables.variableAtIndex(k)[0]] = 10**flux[k]
                            F.append(designspacetoolbox_test.DSExpressionEvaluateWithVariablePool(expr, Var._data))
                        if zlim == None:
                            self.Zlim = [np.amin(F), np.amax(F)]
                            zlim=self.Zlim
                        elif zlim != self.Zlim:
                            if min([zlim[0], np.amin(F)]) != zlim[0]:
                                zlim[0] = np.amin(F)
                            if max([zlim[1], np.amax(F)]) != zlim[1]:
                                zlim[1] = np.amax(F)
                        Z = matplotlib.mlab.griddata(V[:,0], V[:,1], np.array(F), X, Y)
                    else:
                        Z = matplotlib.mlab.griddata(V[:,0], V[:,1], np.repeat(1, len(V[:,0])), X, Y)
                        for (i, yi) in enumerate(y):
                            self.Xi[self.Variables['Y']] = 10**y[i]
                            Var[self.Variables['Y']] = 10**y[i]
                            for (j, xj) in enumerate(x):
                                self.Xi[self.Variables['X']] = 10**xj
                                Var[self.Variables['X']] = 10**xj
                                SS = case.steadyStateAtPoint(self.Xi)
                                flux = case.steadyStateFluxAtPoint(self.Xi)
                                for k in xrange(len(SS)):
                                    Var[self.Dspace.dependentVariables.variableAtIndex(k)[0]] = 10**SS[k]
                                    Var['V_' + self.Dspace.dependentVariables.variableAtIndex(k)[0]] = 10**flux[k]
                                function_eval = designspacetoolbox_test.DSExpressionEvaluateWithVariablePool(expr, Var._data)
                                Z[i,j] *= function_eval
                            if zlim == None:
                                zlim = [np.amin(Z), np.amax(Z)]
                            elif zlim != self.Zlim:
                                if min([zlim[0], np.amin(Z)]) != zlim[0]:
                                    zlim[0] = np.amin(Z)
                                if max([zlim[1], np.amax(Z)]) != zlim[1]:
                                    zlim[1] = np.amax(Z)
                if contourf == False:
                    cs=matplotlib.pyplot.pcolor(X,
                                                Y,
                                                Z,
                                                cmap=matplotlib.cm.jet,
                                                hold=True)
                else:
                    cs=matplotlib.pyplot.contourf(X,
                                                  Y,
                                                  Z,
                                                  cmap=matplotlib.cm.jet,
                                                  hold=True,
                                                  levels=np.linspace(np.amin(Z), np.amax(Z), maxNumPoints))
                contours.append(cs)
                if zlim == None:
                    zlim = [np.amin(Z), np.amax(Z)]
                elif zlim != self.Zlim:
                    if min([zlim[0], np.amin(Z)]) != zlim[0]:
                        zlim[0] = np.amin(Z)
                    if max([zlim[1], np.amax(Z)]) != zlim[1]:
                        zlim[1] = np.amax(Z)
                cs.set_clim([zlim[0], zlim[1]])
                cs.vmin=zlim[0]
                cs.vmax=zlim[1]
                
        def _plot2DFunction(self, lower, upper, colorbar, function, resolution, isLogLinear=False, contourf=False):
                cases = self.Dspace.validCases(lower=lower, upper=upper)
                keys = [str(case.caseNumber) for case in cases]
                Var = self.Xi.copy()
                designspacetoolbox_test.DSVariablePoolSetReadWriteAdd(Var._data)
                contours=list()
                expr = designspacetoolbox_test.DSExpressionByParsingString(function)
                ax=matplotlib.pyplot.gca()  
                zlim = self.Zlim
                for case in cases:
                        V=case.verticesFor2DSlice(self.Xi,
                                                  self.Variables['X'],
                                                  self.Variables['Y'],
                                                  self.Limits['X'],
                                                  self.Limits['Y'])
                        if V != []:
                                Vx=np.sort(V[:,0])
                                Vy=np.sort(V[:,1])
                                xNumPoint = (max(Vx)-min(Vx)+2)*resolution/(math.log10(max(self.Limits['X']))-math.log10(min(self.Limits['X'])));
                                yNumPoint = (max(Vy)-min(Vy)+2)*resolution/(math.log10(max(self.Limits['Y']))-math.log10(min(self.Limits['Y'])));
                                maxNumPoints = max([xNumPoint, yNumPoint])
                                x, y = self._generateMeshForRegion(V, resolution)
                                X,Y = np.meshgrid(x, y)
                                if isLogLinear == True:
                                        F = list()
                                        for i in xrange(len(V[:,0])):
                                                self.Xi[self.Variables['X']] = 10**V[i,0]
                                                self.Xi[self.Variables['Y']] = 10**V[i,1]
                                                Var[self.Variables['X']] = 10**V[i,0]
                                                Var[self.Variables['Y']] = 10**V[i,1]
                                                SS = case.steadyStateAtPoint(self.Xi)
                                                flux = case.steadyStateFluxAtPoint(self.Xi)
                                                for k in xrange(len(SS)):
                                                        Var[self.Dspace.dependentVariables.variableAtIndex(k)[0]] = 10**SS[k]
                                                        Var['V_' + self.Dspace.dependentVariables.variableAtIndex(k)[0]] = 10**flux[k]
                                                F.append(designspacetoolbox_test.DSExpressionEvaluateWithVariablePool(expr, Var._data))
                                        if zlim == None:
                                                zlim = [np.amin(F), np.amax(F)]
                                        elif zlim != self.Zlim:
                                                if min([zlim[0], np.amin(F)]) != zlim[0]:
                                                        zlim[0] = np.amin(F)
                                                if max([zlim[1], np.amax(F)]) != zlim[1]:
                                                        zlim[1] = np.amax(F)
                                        Z = matplotlib.mlab.griddata(V[:,0], V[:,1], np.array(F), X, Y)
                                else:
                                        Z = matplotlib.mlab.griddata(V[:,0], V[:,1], np.repeat(1, len(V[:,0])), X, Y)
                                        for (i, yi) in enumerate(y):
                                                self.Xi[self.Variables['Y']] = 10**y[i]
                                                Var[self.Variables['Y']] = 10**y[i]
                                                for (j, xj) in enumerate(x):
                                                        self.Xi[self.Variables['X']] = 10**xj
                                                        Var[self.Variables['X']] = 10**xj
                                                        SS = case.steadyStateAtPoint(self.Xi)
                                                        flux = case.steadyStateFluxAtPoint(self.Xi)
                                                        for k in xrange(len(SS)):
                                                                Var[self.Dspace.dependentVariables.variableAtIndex(k)[0]] = 10**SS[k]
                                                                Var['V_' + self.Dspace.dependentVariables.variableAtIndex(k)[0]] = 10**flux[k]
                                                        function_eval = designspacetoolbox_test.DSExpressionEvaluateWithVariablePool(expr, Var._data)
                                                        Z[i,j] *= function_eval
                                                if zlim == None:
                                                        zlim = [np.amin(Z), np.amax(Z)]
                                                elif zlim != self.Zlim:
                                                        if min([zlim[0], np.amin(Z)]) != zlim[0]:
                                                                zlim[0] = np.amin(Z)
                                                        if max([zlim[1], np.amax(Z)]) != zlim[1]:
                                                                zlim[1] = np.amax(Z)
                                if contourf == False:
                                        cs=matplotlib.pyplot.pcolor(X,
                                                                    Y,
                                                                    Z,
                                                                    cmap=matplotlib.cm.jet,
                                                                    hold=True)
                                else:
                                         cs=matplotlib.pyplot.contourf(X,
                                                                       Y,
                                                                       Z,
                                                                       cmap=matplotlib.cm.jet,
                                                                       hold=True,
                                                                       levels=np.linspace(np.amin(Z), np.amax(Z), maxNumPoints))
                                contours.append(cs)
                                if zlim == None:
                                        zlim = [np.amin(Z), np.amax(Z)]
                                elif zlim != self.Zlim:
                                        if min([zlim[0], np.amin(Z)]) != zlim[0]:
                                                zlim[0] = np.amin(Z)
                                        if max([zlim[1], np.amax(Z)]) != zlim[1]:
                                                zlim[1] = np.amax(Z)
                for i in contours:
                        i.set_clim([zlim[0], zlim[1]])
                        i.vmin=zlim[0]
                        i.vmax=zlim[1]                  
                designspacetoolbox_test.DSExpressionFree(expr)
                if colorbar == True:
                        axis = matplotlib.pyplot.gca()
                        ax, _ = matplotlib.colorbar.make_axes(axis)
                        self._functionColorbar(ax, zlim)
                        matplotlib.pyplot.sca(axis)
                        
        def _plot2DRouth(self, algebraic, lower, upper, colorbar, resolution, levels=None, contourf=True):
                cases = self.Dspace.validCases(lower=lower, upper=upper)
                ssystems = list()
                keys = dict()
                x=np.linspace(math.log10(lower[self.Variables['X']]),
                              math.log10(upper[self.Variables['X']]),
                              resolution)
                y=np.linspace(math.log10(lower[self.Variables['Y']]),
                              math.log10(upper[self.Variables['Y']]),
                              resolution)
                routh = np.zeros((resolution, resolution))
                Var = self.Xi.copy()
                for k in cases:
                    ssystems.append(designspacetoolbox_test.DSSSystemByRemovingAlgebraicConstraints(
                                     designspacetoolbox_test.DSCaseSSystem(k._data), 
                                     algebraic._data))
                for i in xrange(len(x)):
                    Var[self.Variables['X']] = 10**x[i]
                    for j in xrange(len(y)):
                        temp = list()
                        validCases=list()
                        Var[self.Variables['Y']] = 10**y[j]
                        for k in xrange(len(cases)):   
                            case = cases[k]           
                            if designspacetoolbox_test.DSCaseIsValidAtPoint(case._data, Var._data) == False:
                                continue
#                            validCases.append(case.caseNumber)
#                            print designspacetoolbox_test.DSSSystemRouthArray(ssystems[k],
#                                                                               Var._data)
                            temp.append(designspacetoolbox_test.DSSSystemRouthIndex(ssystems[k],
                                         Var._data))
                        if len(temp) == 0:
                            routh[i,j] = -1.
                            continue
                        routh[i,j] = max(temp)
                        keys[max(temp)] = max(temp)
                X, Y = np.meshgrid(x, y)
                print keys.keys()
                for k in ssystems:
                    designspacetoolbox_test.DSSSystemFree(k)
                if levels == None:
                    levels = keys.keys()
                    levels.append(2**(len(self.Dspace.dependentVariables)-len(algebraic)+1))
                    levels = np.sort([i-0.5 for i in levels])
                cs=matplotlib.pyplot.contourf(X,
                                              Y,
                                              routh.T,
                                              cmap=matplotlib.cm.jet,
                                              levels=levels,
                                              hold=True)
                if colorbar == True:
                    axis = matplotlib.pyplot.gca()
                    matplotlib.pyplot.colorbar()
                                                  
        def plot2DSliceWithSteadyStatesAsHeatmap(self, point, xVariable, yVariable, zVariable, xRange, yRange, resolution=500, zRange=[]):
                lower = point.copy()
                upper = point.copy()
                lower[xVariable] = min(xRange)
                lower[yVariable] = min(yRange)
                upper[xVariable] = max(xRange)
                upper[yVariable] = max(yRange)
                C=self.validCases(lower=lower,upper=upper)
                Xi=list()
                Yi=list()
                Zi=list()
                for i in xrange(len(C)):
                        V=C[i].verticesFor2DSliceWithSteadyStates(point, xVariable, yVariable, zVariable, xRange, yRange)
                        Vx=np.sort(V[:,0])
                        Vy=np.sort(V[:,1])
                        xNumPoint = (max(Vx)-min(Vx))*resolution/(math.log10(max(xRange))-math.log10(min(xRange)));
                        yNumPoint = (max(Vy)-min(Vy))*resolution/(math.log10(max(yRange))-math.log10(min(yRange)));
                        x=np.linspace(Vx[0], Vx[1], int(xNumPoint/(len(Vx)-1)))
                        y=np.linspace(Vy[0], Vy[1], int(yNumPoint/(len(Vy)-1)))
                        for j in xrange(1, len(Vx)-1):
                                x=np.append(x, np.linspace(Vx[j], Vx[j+1], int(xNumPoint/(len(Vx)-1))))
                                y=np.append(y, np.linspace(Vy[j], Vy[j+1], int(yNumPoint/(len(Vy)-1))))
                        X,Y = np.meshgrid(x, y)
                        Xi.append(X)
                        Yi.append(Y)
                        Zi.append(matplotlib.mlab.griddata(V[:,0], V[:,1], V[:,2], X, Y))
                        if len(zRange) == 0:
                                zRange = [min(V[:,2]), max(V[:,2])]
                        if min(V[:,2]) < zRange[0]:
                                zRange[0] = min(V[:,2])
                        if max(V[:,2]) > zRange[1]:
                                zRange[1] = max(V[:,2])
                for i in xrange(len(C)):
                        if len(Zi[i]) == 0:
                                continue
                        matplotlib.pyplot.contourf(Xi[i], Yi[i], Zi[i], cmap=matplotlib.cm.jet, levels=np.linspace(zRange[0], zRange[1], 200))
                        matplotlib.pyplot.hold(True)
                matplotlib.pyplot.hold(False)
                matplotlib.pyplot.xlim([math.log10(min(xRange)),
                                        math.log10(max(xRange))])
                matplotlib.pyplot.ylim([math.log10(min(yRange)),
                                        math.log10(max(yRange))])
                matplotlib.pyplot.xlabel('$\log_{10}[$' + xVariable + '$]$')
                matplotlib.pyplot.ylabel('$\log_{10}[$' + yVariable + '$]$')
        def plot2DSliceWithSteadyStateFluxes(self, point, xVariable, yVariable, zVariable, xRange, yRange):
                lower = point.copy()
                upper = point.copy()
                lower.setValueForVariables([xVariable, yVariable], [min(xRange), min(yRange)])
                upper.setValueForVariables([xVariable, yVariable], [max(xRange), max(yRange)])
                ax=Axes3D.Axes3D(matplotlib.pyplot.gcf())
                C=self.validCases(lower=lower,upper=upper)
                cm=matplotlib.colors.Normalize(vmin=0, vmax=len(C))
                allVertices = list()
                allColors = list()
                mi = None
                ma = None
                for i in xrange(0, len(C)):
                        V=C[i].verticesFor2DSliceWithSteadyStates(point, xVariable, yVariable, zVariable, xRange, yRange)
                        if V != None:
                                color = matplotlib.colors.rgb2hex(matplotlib.cm.hsv(cm(i)))
                                allVertices.append(V)
                                allColors.append(color)
                                if mi == None:
                                        mi = min(V[:,2])
                                elif mi > min(V[:,2]):
                                        mi = min(V[:,2])
                                if ma == None:
                                        ma = max(V[:,2])
                                elif ma < max(V[:,2]):
                                        ma = max(V[:,2])
                P = art3d.Poly3DCollection(allVertices, facecolors=allColors)
                ax.add_collection3d(P)
                ax.set_zlim3d([mi, ma])                 
                ax.set_zlabel('$\log_{10}[V_{' + zVariable + '}]$')
                matplotlib.pyplot.xlim([math.log10(min(xRange)),
                                        math.log10(max(xRange))])
                matplotlib.pyplot.ylim([math.log10(min(yRange)),
                                        math.log10(max(yRange))])
                matplotlib.pyplot.xlabel('$\log_{10}[$' + xVariable + '$]$')
                matplotlib.pyplot.ylabel('$\log_{10}[$' + yVariable + '$]$')        
        
class DesignSpace:
        """ A python class of the DSVariablePool object"""
        dependentVariables = None
        def __init__(self, dependentVariables, stringList):
                self.dependentVariables = dependentVariables
                self.__parseStrings__(dependentVariables, stringList)
        def __del__(self):
                if hasattr(self, '_data')==0:
                        return
                if hasattr(self, '__is_constant__') == True:
                        return
                if self._data==None:
                        return
                designspacetoolbox_test.DSDesignSpaceFree(self._data)
        def __repr__(self):
                if self._data != None:
                        designspacetoolbox_test.DSDesignSpacePrint(self._data)
                return ''
        def __parseStrings__(self, dependentVariables, stringList):
                if isinstance(dependentVariables,VariablePool)==0:
                        return
                if type(stringList)!=list:
                        return
                if hasattr(self, '_data')==0:
                        self._data=designspacetoolbox_test.DSDesignSpaceByParsingStrings(dependentVariables._data, stringList, len(dependentVariables))
        def caseWithCaseNumber(self, caseNumber):
                caseNumber = long(caseNumber)
                if hasattr(self, '_data')==0:
                        return None
                aCase=Case()
                aCase._data = designspacetoolbox_test.DSDesignSpaceCaseWithCaseNumber(self._data, caseNumber)
                return aCase
        def validCases(self, lower=None, upper=None):
                if hasattr(self, '_data')==0:
                        return None
                if self.numberOfValidCases==0:
                        return None
                if lower == None and upper == None:
                        temp=designspacetoolbox_test.DSDesignSpaceCalculateAllValidCases(self._data)
                        numberValid=self.numberOfValidCases
                        dict=designspacetoolbox_test.DSDictionaryFromArray(temp, numberValid)
                        cases = list()
                        for i in xrange(numberValid):
                                aCase=Case()
                                aCase._data = designspacetoolbox_test.DSDictionaryValueForName(dict, str(i))
                                aCase._data = designspacetoolbox_test.DSSWIGVoidAsCase(aCase._data)
                                cases.append(aCase)
                        designspacetoolbox_test.DSSecureFree(temp)
                        designspacetoolbox_test.DSDictionaryFree(dict)
                        return cases
                elif lower != None and upper != None:
                        if isinstance(lower, VariablePool) == False:
                                return None
                        if isinstance(upper, VariablePool) == False:
                                return none
                        dict=designspacetoolbox_test.DSDesignSpaceCalculateAllValidCasesForSlice(self._data, 
                                                                                                 lower._data,
                                                                                                 upper._data)
                        if dict == None:
                                return None
                        cases = list()
                        for i in xrange(1, designspacetoolbox_test.DSDesignSpaceNumberOfCases(self._data)+1):
                                if designspacetoolbox_test.DSDictionaryValueForName(dict, str(i)) != None:
                                        aCase=Case()
                                        aCase._data = designspacetoolbox_test.DSDictionaryValueForName(dict, str(i))
                                        aCase._data = designspacetoolbox_test.DSSWIGVoidAsCase(aCase._data)
                                        cases.append(aCase)
                        designspacetoolbox_test.DSDictionaryFree(dict)
                        return cases
        @property
        def numberOfEquations(self):
                if hasattr(self, '_data')==0:
                        return None
                return designspacetoolbox_test.DSDesignSpaceNumberOfEquations(self._data)
        def findIntersections(self, cases, lower=None, upper=None, n=2):
                if type(n)==int or type(n) == long:
                        n = [n]
                if len(cases)<=1:
                        return None
                intersections = list()
                for i in xrange(len(n)):
                        if len(cases)<n[i]:
                                break
                        current=list()
                        validIndices=list()
                        comb = itertools.combinations(range(len(cases)), n[i])
                        for j in comb:
                                C=[cases[k]._data for k in j]
                                if lower == None and upper == None:
                                        if designspacetoolbox_test.DSCaseIntersectionIsValid(n[i], C)==True:
                                                current.append([cases[k] for k in j])
                                                [validIndices.append(k) for k in j]
                                else:
                                        if designspacetoolbox_test.DSCaseIntersectionIsValidAtSlice(n[i], C, lower._data, upper._data)==True:
                                                current.append([cases[k] for k in j])
                                                [validIndices.append(k) for k in j]
                        current.reverse()
                        if len(current) != 0:
                                intersections.append(current)
                        else: 
                                break
                        cases=[cases[j] for j in np.unique(validIndices)]
                return intersections
        def verticesForIntersection(self, cases, lower=None, upper=None):
                n = len(cases)
                if n <= 1:
                        return None
        def calculateUnderdeterminedCases(self):
                designspacetoolbox_test.DSDesignSpaceCalculateUnderdeterminedCases(self._data)
        def subcaseForCaseWithCaseNumber(self, caseNumber):
                subcaseDictionary = designspacetoolbox_test.DSDesignSpaceSubcaseDictionary(self._data)
                if subcaseDictionary == None:
                        return None
                subcase = designspacetoolbox_test.DSDictionaryValueForName(subcaseDictionary, str(caseNumber))
                if subcase == None:
                        return None
                subcase = designspacetoolbox_test.DSSWIGVoidAsSubcase(subcase)
                internalData = designspacetoolbox_test.DSSubcaseInternalDesignSpace(subcase)
                ds = DesignSpace(1, 1)
                ds._data = internalData
                ds.__is_constant__ = True
                return ds
        @property
        def numberOfCases(self):
                if hasattr(self, '_data')==0:
                        return 0
                return designspacetoolbox_test.DSDesignSpaceNumberOfCases(self._data)
        @property
        def numberOfValidCases(self):
                if hasattr(self, '_data')==0:
                        return None
                return designspacetoolbox_test.DSDesignSpaceNumberOfValidCases(self._data)
        @property
        def equations(self):
                if hasattr(self, '_data')==0:
                        return None
                if self.numberOfEquations == 0:
                        return None
                temp=designspacetoolbox_test.DSDesignSpaceEquations(self._data)
                numberEquations=self.numberOfEquations
                dict=designspacetoolbox_test.DSDictionaryFromArray(temp, numberEquations)
                cases = list()
                for i in xrange(numberEquations):
                        k = designspacetoolbox_test.DSDictionaryValueForName(dict, str(i))
                        k = designspacetoolbox_test.DSSWIGVoidAsExpression(k)
                        strval = designspacetoolbox_test.DSExpressionAsString(k);
                        cases.append(strval);
                        designspacetoolbox_test.DSExpressionFree(k)
                designspacetoolbox_test.DSSecureFree(temp)
                designspacetoolbox_test.DSDictionaryFree(dict)
                return cases
        @property
        def independentVariables(self):
                if hasattr(self, '_data') == False:
                        return None
                Xi = VariablePool()
                t = designspacetoolbox_test.DSDesignSpaceXi(self._data)
                Xi._data = designspacetoolbox_test.DSVariablePoolCopy(t)
                designspacetoolbox_test.DSVariablePoolSetReadWrite(Xi._data)
                return Xi

        
class Case:
        def __del__(self):
                if hasattr(self, '_data')==0:
                        return
                if self._data==None:
                        return
                designspacetoolbox_test.DSCaseFree(self._data)
        def __repr__(self):
                return '<DSCase object> with Case #: ' + str(self.caseNumber)
        @property
        def numberOfEquations(self):
                if hasattr(self, '_data')==0:
                        return None
                return designspacetoolbox_test.DSCaseNumberOfEquations(self._data)
        @property
        def numberOfConditions(self):
                if hasattr(self, '_data')==0:
                        return None
                return designspacetoolbox_test.DSCaseNumberOfConditions(self._data)                
        @property
        def independentVariables(self):
                if hasattr(self, '_data') == False:
                        return None
                Xi = VariablePool()
                t = designspacetoolbox_test.DSCaseSSystem(self._data)
                t = designspacetoolbox_test.DSSSystemXi(t)
                Xi._data = designspacetoolbox_test.DSVariablePoolCopy(t)
                designspacetoolbox_test.DSVariablePoolSetReadWrite(Xi._data)
                return Xi
        @property
        def dependentVariables(self):
                if hasattr(self, '_data') == False:
                        return None
                Xd = VariablePool()
                t = designspacetoolbox_test.DSCaseSSystem(self._data)
                t = designspacetoolbox_test.DSSSystemXd(t)
                Xd._data = designspacetoolbox_test.DSVariablePoolCopy(t)
                designspacetoolbox_test.DSVariablePoolSetReadWrite(Xd._data)
                return Xd
        def equations(self):
                if hasattr(self, '_data')==0:
                        return None
                if self.numberOfEquations == 0:
                        return None
                temp=designspacetoolbox_test.DSCaseEquations(self._data)
                numberEquations=self.numberOfEquations
                dict=designspacetoolbox_test.DSDictionaryFromArray(temp, numberEquations)
                equations = list()
                for i in xrange(numberEquations):
                        k = designspacetoolbox_test.DSDictionaryValueForName(dict, str(i))
                        k = designspacetoolbox_test.DSSWIGVoidAsExpression(k)
                        st = designspacetoolbox_test.DSExpressionAsString(k)
                        equations.append(st)
                        designspacetoolbox_test.DSExpressionFree(k)
                designspacetoolbox_test.DSSecureFree(temp)
                designspacetoolbox_test.DSDictionaryFree(dict)
                return equations
        def solution(self, scale='linear'):
                if hasattr(self, '_data')==0:
                        return None
                if self.numberOfEquations == 0:
                        return None
                if scale.upper()=='LINEAR':
                        temp=designspacetoolbox_test.DSCaseSolution(self._data)
                elif scale.upper()=='LOGARITHMIC': 
                        temp=designspacetoolbox_test.DSCaseLogarithmicSolution(self._data)
                else: 
                        return None
                if temp == None:
                    return None
                numberEquations=self.numberOfEquations
                dict=designspacetoolbox_test.DSDictionaryFromArray(temp, numberEquations)
                solution = list()
                for i in xrange(numberEquations):
                        k = designspacetoolbox_test.DSDictionaryValueForName(dict, str(i))
                        k = designspacetoolbox_test.DSSWIGVoidAsExpression(k)
                        st = designspacetoolbox_test.DSExpressionAsString(k)
                        solution.append(st)
                        designspacetoolbox_test.DSExpressionFree(k)
                designspacetoolbox_test.DSSecureFree(temp)
                designspacetoolbox_test.DSDictionaryFree(dict)
                return solution
        def conditions(self, scale='linear', type='symbolic'):
                """ A python class of the DSVariablePool object"""
                if hasattr(self, '_data')==0:
                        return None
                if self.numberOfEquations == 0:
                        return None
                if scale.upper()=='LINEAR':
                        temp=designspacetoolbox_test.DSCaseConditions(self._data)
                elif scale.upper()=='LOGARITHMIC': 
                        temp=designspacetoolbox_test.DSCaseLogarithmicConditions(self._data)
                else:
                        return None
                numberOfConditions=self.numberOfConditions
                dict=designspacetoolbox_test.DSDictionaryFromArray(temp, numberOfConditions)
                conditions = list()
                for i in xrange(numberOfConditions):
                        k = designspacetoolbox_test.DSDictionaryValueForName(dict, str(i))
                        k = designspacetoolbox_test.DSSWIGVoidAsExpression(k)
                        conditions.append(designspacetoolbox_test.DSExpressionAsString(k))
                        designspacetoolbox_test.DSExpressionFree(k)
                designspacetoolbox_test.DSSecureFree(temp)
                designspacetoolbox_test.DSDictionaryFree(dict)
                return conditions
        def boundaries(self, scale='linear', eval=None):
                """ A python class of the DSVariablePool object"""
                if hasattr(self, '_data')==0:
                        return None
                if self.numberOfEquations == 0:
                        return None
                if scale.upper()=='LINEAR':
                        temp=designspacetoolbox_test.DSCaseBoundaries(self._data)
                elif scale.upper()=='LOGARITHMIC': 
                        temp=designspacetoolbox_test.DSCaseLogarithmicBoundaries(self._data)
                else:
                        return None
                if temp == None:
                        return None
                numberOfConditions=self.numberOfConditions
                dict=designspacetoolbox_test.DSDictionaryFromArray(temp, numberOfConditions)
                boundaries = list()
                for i in xrange(numberOfConditions):
                        k = designspacetoolbox_test.DSDictionaryValueForName(dict, str(i))
                        k = designspacetoolbox_test.DSSWIGVoidAsExpression(k)
                        if eval == None:
                                boundaries.append(designspacetoolbox_test.DSExpressionAsString(k))
                        else:
                                boundaries.append(designspacetoolbox_test.DSExpressionEvaluateWithVariablePool(k, eval._data))
                        designspacetoolbox_test.DSExpressionFree(k)
                designspacetoolbox_test.DSSecureFree(temp)
                designspacetoolbox_test.DSDictionaryFree(dict)
                return boundaries
        def boundsForVariable(self, variable, lower=None, upper=None):
                variables = self.independentVariables
                if isinstance(variables, VariablePool)==False:
                        return None
                if variables.has_key(variable)==False:
                        return None
                if self._data == None:
                        return None
                if lower == None and upper == None:
                    bounds = designspacetoolbox_test.DSCaseBoundingRangeForVariable(self._data, variable)
                elif isinstance(lower, VariablePool) == True and isinstance(upper, VariablePool):
                    bounds = designspacetoolbox_test.DSCaseBoundingRangeForVariableWithConstraints(self._data,
                                                                                                   variable,
                                                                                                   lower._data,
                                                                                                   upper._data)
                return np.array(bounds)
        def verticesFor1DSlice(self, point, xVariable, xRange):
                if isinstance(point, VariablePool)==False:
                        return None
                if point.has_key(xVariable)==False:
                        return None
                if self._data == None:
                        return None
                lower = point.copy()
                upper = point.copy()
                lower[xVariable] = min(xRange)
                upper[xVariable] = max(xRange)
                vertices=designspacetoolbox_test.DSCaseVerticesFor1DSlice(self._data, 
                                                                          lower._data,
                                                                          upper._data,
                                                                          xVariable)
                lower = None
                upper = None
                return np.asarray(vertices)
        def verticesFor2DSlice(self, point, xVariable, yVariable, xRange, yRange):
                if isinstance(point, VariablePool)==False:
                        return None
                if point.has_key(xVariable)==False:
                        return None
                if point.has_key(yVariable)==False:
                        return None
                if self._data == None:
                        return None
                lower = point.copy()
                upper = point.copy()
                lower[xVariable] = min(xRange)
                lower[yVariable] = min(yRange)
                upper[xVariable] = max(xRange)
                upper[yVariable] = max(yRange)
                vertices=designspacetoolbox_test.DSCaseVerticesFor2DSlice(self._data, 
                                                                          lower._data,
                                                                          upper._data,
                                                                          xVariable,
                                                                          yVariable)
                lower = None
                upper = None
                return np.asarray(vertices)
        def traingulatedMeshFor2DSlice(self, point, xVariable, yVariable, xRange, yRange):
                V = self.verticesFor2DSlice(point, xVariable, yVariable, xRange, yRange)
                Vx = V[:,0]
                Vy = V[:,1]
                if Vy[0] == Vy[1]:
                        x = np.linspace(Vx[0], Vx[1], 10)
                        y = 0*x + Vy[0]
                elif Vx[0] == Vx[1]:
                        y = np.linspace(Vy[0], Vy[1], 10)
                        x = 0*y + Vx[0]
                else:
                        x = np.linspace(Vx[0], Vx[1], 10)
                        m = (Vy[0]-Vy[1])/(Vx[0]-Vx[1])
                        C = Vy[0]-m*Vx[0]
                        y = m*x+C
                for i in xrange(1, len(Vx)-1):
                        if Vy[i] == Vy[i+1]:
                                x = np.append(x, np.linspace(Vx[i], Vx[i+1], 10))
                                y = np.append(y, 0*np.linspace(Vx[i], Vx[i+1], 10) + Vy[i])
                        elif Vx[i] == Vx[i+1]:
                                y = np.append(y,np.linspace(Vy[i], Vy[i+1], 10))
                                x = np.append(x, 0*np.linspace(Vy[i], Vy[i+1], 10) + Vx[0])
                        else:
                                x = np.append(x, np.linspace(Vx[i], Vx[i+1], 10))
                                m = (Vy[i]-Vy[i+1])/(Vx[i]-Vx[i+1])
                                C = Vy[i]-m*Vx[i]
                                y = np.append(y, m*np.linspace(Vx[i], Vx[i+1], 10)+C)
                return [x, y]
        def verticesFor2DSliceWithSteadyStates(self, point, xVariable, yVariable, zVariable, xRange, yRange):
                if isinstance(point, VariablePool)==False:
                        return None
                if point.has_key(xVariable)==False:
                        return None
                if point.has_key(yVariable)==False:
                        return None
                if self._data == None:
                        return None
                Xd = designspacetoolbox_test.DSSSystemXd(designspacetoolbox_test.DSCaseSSystem(self._data))
                index = designspacetoolbox_test.DSVariablePoolIndexOfVariableWithName(Xd, zVariable)
                if index >= self.numberOfEquations:
                        return None
                lower = point.copy()
                upper = point.copy()
                lower[xVariable] = min(xRange)
                lower[yVariable] = min(yRange)
                upper[xVariable] = max(xRange)
                upper[yVariable] = max(yRange)
                vertices=designspacetoolbox_test.DSCaseVerticesFor2DSlice(self._data, 
                                                                          lower._data,
                                                                          upper._data,
                                                                          xVariable,
                                                                          yVariable)
                if vertices == []:
                        return None
                vertices=np.asarray(vertices)
                SS = list()
                for i in xrange(0, len(vertices)):
                        lower.setValueForVariables([xVariable, yVariable], [10**vertices[i, 0], 10**vertices[i, 1]])
                        temp = self.steadyStateAtPoint(lower)
                        SS.append(temp[index, 0])
                SS = np.resize(np.asarray(SS), (len(SS), 1))
                vertices = np.append(vertices, SS, 1)
                return vertices
        def verticesFor2DSliceWithSteadyStateFluxes(self, point, xVariable, yVariable, zVariable, xRange, yRange):
                if isinstance(point, VariablePool)==False:
                        return None
                if point.has_key(xVariable)==False:
                        return None
                if point.has_key( yVariable)==False:
                        return None
                if self._data == None:
                                return None
                Xd = designspacetoolbox_test.DSSSystemXd(designspacetoolbox_test.DSCaseSSystem(self._data))
                index = designspacetoolbox_test.DSVariablePoolIndexOfVariableWithName(Xd, zVariable)
                if index >= self.numberOfEquations:
                        return None
                lower = point.copy()
                upper = point.copy()
                lower[xVariable] = min(xRange)
                lower[yVariable] = min(yRange)
                upper[xVariable] = max(xRange)
                upper[yVariable] = max(yRange)
                vertices=designspacetoolbox_test.DSCaseVerticesFor2DSlice(self._data, 
                                                                          lower._data,
                                                                          upper._data,
                                                                          xVariable,
                                                                          yVariable)
                if vertices == []:
                        return None
                vertices=np.asarray(vertices)
                flux = list()
                for i in xrange(0, len(vertices)):
                        lower.setValueForVariables([xVariable, yVariable], [10**vertices[i, 0], 10**vertices[i, 1]])
                        temp = self.steadyStateFluxAtPoint(lower)
                        flux.append(temp[index, 0])
                flux = np.resize(np.asarray(flux), (len(flux), 1))
                vertices = np.append(vertices, flux, 1)
                return vertices
        def validParameterSet(self, lower=None, upper=None):
                if hasattr(self, '_data')==0:
                        return None
                if self.numberOfEquations == 0:
                        return None
                P=VariablePool()
                if lower == None or upper == None:
                        P._data = designspacetoolbox_test.DSCaseValidParameterSet(self._data)
                else:
                        if isinstance(lower, VariablePool) == False:
                                return None
                        if isinstance(upper, VariablePool) == False:
                                return none
                        P._data = designspacetoolbox_test.DSCaseValidParameterSetAtSlice(self._data, lower._data, upper._data)
                return P
        def steadyStateAtPoint(self, Xi0):                
                """ A python class of the DSVariablePool object"""
                if hasattr(self, '_data')==0:
                        return None
                if self.numberOfEquations == 0:
                        return None
                ssys = designspacetoolbox_test.DSCaseSSystem(self._data)
                ss=designspacetoolbox_test.DSSSystemSteadyStateValues(ssys, Xi0._data)
                ss=np.asarray(ss)
                return ss
        def steadyStateFluxAtPoint(self, Xi0):                
                """ A python class of the DSVariablePool object"""
                if hasattr(self, '_data')==0:
                        return None
                if self.numberOfEquations == 0:
                        return None
                ssys = designspacetoolbox_test.DSCaseSSystem(self._data)
                ss=designspacetoolbox_test.DSSSystemSteadyStateFlux(ssys, Xi0._data)
                ss=np.asarray(ss)
                return ss
        def logarithmicGain(self, XdName, XiName):
                """ A python class of the DSVariablePool object"""
                if hasattr(self, '_data')==0:
                        return None
                if self.numberOfEquations == 0:
                        return None
                logGain = designspacetoolbox_test.DSCaseLogarithmicGain(self._data, XdName, XiName)
                return logGain
#                ssys = designspacetoolbox_test.DSCaseSSystem(self._data)
#                ss=designspacetoolbox_test.DSSSystemSteadyStateFlux(ssys, Xi0._data)
#                ss=np.asarray(ss)
#                return ss
        @property
        def caseNumber(self):
                return designspacetoolbox_test.DSCaseNumber(self._data)


